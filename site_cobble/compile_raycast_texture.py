import cobble

class RaycastTextureConverter(cobble.Target):
  def __init__(self, loader, package, name,
               environment,
               tex_name):
    super(RaycastTextureConverter, self).__init__(loader, package, name)
    self.environment = environment
    self.tex_name = tex_name
    self.leaf = True

  def _derive_local(self, unused):
    return self.package.project.named_envs[self.environment]

  def _using_and_products(self, env_local): 
    pnm = self.package.inpath(self.tex_name + '.pnm')
    header, source = [self.package.genpath(self.tex_name + '.' + ext)
                           for ext in ['h', 'cc']]

    script = self.project.inpath('demo', 'raycast', 'process_textures.rb')
    converter = {
      'outputs': [header, source],
      'rule': 'convert_raycast_texture',
      'inputs': [pnm],
      'implicit': [script],
      'variables': {
        'script': script,
        'outputdir': self.package.genpath(),
        'name': self.tex_name,
      },
    }

    using = cobble.env.make_appending_delta(
      __order_only__ = [ header ],
      cxx_flags = [ '-I' + self.project.genpath() ],
    )

    return (using, [converter])


package_verbs = {
  'convert_raycast_texture': RaycastTextureConverter,
}

ninja_rules = {
  'convert_raycast_texture': {
    'command': '$script $in $outputdir $name',
    'description': 'TEX $in',
  },
}
