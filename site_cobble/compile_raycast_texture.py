import cobble.target
from cobble.plugin import target_def
import cobble.env

TEX_NAME = cobble.env.overrideable_string_key('raycast_tex_name')
TEX_SCRIPT = cobble.env.overrideable_string_key('raycast_tex_script')
KEYS = frozenset([TEX_NAME, TEX_SCRIPT])

@target_def
def convert_raycast_texture(package, name, /, *,
        env,
        tex_name,
        deps = []):

    def mku(ctx):
        script = package.project.inpath('demo', 'raycast', 'process_textures.rb')
        gen_env = ctx.env.subset([]).derive({
            TEX_NAME.name: tex_name,
            TEX_SCRIPT.name: script,
        })

        pnm = package.inpath(tex_name + '.pnm')
        header, source = [package.outpath(gen_env, tex_name + '.' + ext)
                for ext in ['h', 'cc']]

        converter = cobble.target.Product(
            env = gen_env,
            outputs = [header, source],
            rule = 'convert_raycast_texture',
            inputs = [pnm],
            implicit = [script],
        )
        converter.expose(name = tex_name + '.cc', path = source)
        converter.expose(name = tex_name + '.h', path = header)

        using = {
            '__order_only__': [header],
            'cxx_flags': ['-I' + package.project.outpath(gen_env)],
        }

        return (using, [converter])

    return cobble.target.Target(
        concrete = True,
        package = package,
        name = name,
        using_and_products = mku,
        down = lambda _: package.project.named_envs[env],
        deps = deps,
    )

ninja_rules = {
  'convert_raycast_texture': {
    'command': '$raycast_tex_script $in $out $raycast_tex_name',
    'description': 'TEX $in -> $raycast_tex_name',
  },
}
