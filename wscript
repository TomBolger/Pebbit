#
# This file is the default set of rules to compile a Pebble application.
#
# Feel free to customize this to your needs.
#
import os.path
import shutil
import subprocess
from waflib import Task
from waflib.TaskGen import after_method, before_method, feature

top = '.'
out = 'build'
MY_DRIVE_ROOT = r'I:\My drive'


def options(ctx):
    ctx.load('pebble_sdk')


def configure(ctx):
    """
    This method is used to configure your build. ctx.load(`pebble_sdk`) automatically configures
    a build for each valid platform in `targetPlatforms`. Platform-specific configuration: add your
    change after calling ctx.load('pebble_sdk') and make sure to set the correct environment first.
    Universal configuration: add your change prior to calling ctx.load('pebble_sdk').
    """
    ctx.load('pebble_sdk')


@feature('js')
@after_method('process_js')
@before_method('make_pbl_bundle')
def minify_js_for_app_bundle(task_gen):
    js_nodes = [node for node in getattr(task_gen, 'js', [])
                if not node.name.endswith('.js.map')]
    uglify = os.path.expanduser('~/.pebble-sdk/SDKs/current/node_modules/uglify-js/bin/uglifyjs')
    minified_nodes = []
    for node in js_nodes:
        if node.name == 'pebble-js-app.js' and os.path.exists(uglify):
            minified = task_gen.bld.path.get_bld().make_node('minified/pebble-js-app.js')
            minified.parent.mkdir()
            task = task_gen.create_task('minify_js', src=node, tgt=minified)
            task.uglify = uglify
            minified_nodes.append(minified)
        else:
            minified_nodes.append(node)
    task_gen.js = minified_nodes


class minify_js(Task.Task):
    def run(self):
        self.generator.bld.cmd_and_log([
            self.uglify,
            self.inputs[0].abspath(),
            '-c',
            '-m',
            '-o',
            self.outputs[0].abspath()
        ])


def copy_latest_pbw_to_my_drive(ctx):
    pbw = ctx.path.get_bld().make_node('Pebbit.pbw').abspath()
    if not os.path.exists(pbw):
        return

    local_targets = ['/mnt/i/My drive', '/mnt/i/My Drive']
    for target in local_targets:
        if os.path.isdir(target):
            shutil.copyfile(pbw, os.path.join(target, 'Pebbit.pbw'))
            ctx.to_log('Copied Pebbit.pbw to {}\n'.format(target))
            return

    powershell = shutil.which('powershell.exe')
    wslpath = shutil.which('wslpath')
    if not powershell or not wslpath:
        ctx.to_log('My Drive copy skipped: Windows drive path unavailable\n')
        return

    try:
        win_pbw = subprocess.check_output([wslpath, '-w', pbw]).decode('utf-8').strip()
        command = (
            "$target = '{}'; "
            "if (Test-Path -LiteralPath $target) {{ "
            "Copy-Item -LiteralPath '{}' -Destination (Join-Path $target 'Pebbit.pbw') -Force "
            "}}"
        ).format(MY_DRIVE_ROOT.replace("'", "''"), win_pbw.replace("'", "''"))
        subprocess.check_call([powershell, '-NoProfile', '-Command', command])
        ctx.to_log('Copied Pebbit.pbw to {}\n'.format(MY_DRIVE_ROOT))
    except Exception as exc:
        ctx.to_log('My Drive copy skipped: {}\n'.format(exc))


def build(ctx):
    ctx.load('pebble_sdk')

    build_worker = os.path.exists('worker_src')
    binaries = []

    cached_env = ctx.env
    for platform in ctx.env.TARGET_PLATFORMS:
        ctx.env = ctx.all_envs[platform]
        ctx.set_group(ctx.env.PLATFORM_NAME)
        app_elf = '{}/pebble-app.elf'.format(ctx.env.BUILD_DIR)
        ctx.pbl_build(source=ctx.path.ant_glob('src/c/**/*.c'), target=app_elf, bin_type='app')

        if build_worker:
            worker_elf = '{}/pebble-worker.elf'.format(ctx.env.BUILD_DIR)
            binaries.append({'platform': platform, 'app_elf': app_elf, 'worker_elf': worker_elf})
            ctx.pbl_build(source=ctx.path.ant_glob('worker_src/c/**/*.c'),
                          target=worker_elf,
                          bin_type='worker')
        else:
            binaries.append({'platform': platform, 'app_elf': app_elf})
    ctx.env = cached_env

    ctx.set_group('bundle')
    ctx.pbl_bundle(binaries=binaries,
                   js=ctx.path.ant_glob(['src/pkjs/index.js',
                                         'src/pkjs/reddit/**/*.js',
                                         'src/common/**/*.js']),
                   js_entry_file='src/pkjs/index.js')
    ctx.add_post_fun(copy_latest_pbw_to_my_drive)
