import cobble.target
from cobble.plugin import target_def
import cobble.env

STLMUNGE = cobble.env.overrideable_string_key('stlmunge')
KEYS = frozenset([STLMUNGE])

@target_def
def compile_stl(package, name, /, *,
        env,
        stl_file,
        deps = []):

    def mku(ctx):
        stl_file_i = ctx.rewrite_sources([stl_file])[0]

        stl_env = ctx.env.derive({
            'stlmunge': 'ruby ' + package.inpath('stlmunge.rb'),
        })

        header, source = [package.outpath(stl_env, 'model.' + ext)
                for ext in ['h', 'cc']]

        product = cobble.target.Product(
            env = stl_env,
            outputs = [header, source],
            rule = 'compile_stl',
            inputs = [stl_file_i],
        )
        product.expose(name = 'model.h', path = header)
        product.expose(name = 'model.cc', path = source)

        using = {
            '__order_only__': [header],
            'cxx_flags': ['-I' + package.project.outpath(stl_env)],
        }
        return (using, [product])

    return cobble.target.Target(
        concrete = True,
        package = package,
        name = name,
        using_and_products = mku,
        down = lambda _: package.project.named_envs[env],
        deps = deps,
    )

ninja_rules = {
  'compile_stl': {
    'command': '$stlmunge $out < $in',
    'description': 'STL $in',
  },
}
