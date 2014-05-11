import cobble

class StlCompiler(cobble.Target):
  def __init__(self, loader, package, name,
               environment,
               stl_file = []):
    super(StlCompiler, self).__init__(loader, package, name)
    self.environment = environment
    self.stl_file = stl_file
    self.leaf = True

  def _derive_local(self, unused):
    return self.package.project.named_envs[self.environment]

  def _using_and_products(self, env_local): 
    header, source = [self.package.genpath('model.' + ext)
                        for ext in ['h', 'cc']]

    script = self.package.inpath('stlmunge.rb')
    compiler = {
      'outputs': [header, source],
      'rule': 'compile_stl',
      'inputs': [self.package.inpath(self.stl_file)],
      'implicit': [script],
      'variables': {
        'stlmunge': 'ruby ' + script,
        'outputdir': self.package.genroot,
      },
    }

    using = cobble.env.make_appending_delta(
      __order_only__ = [ header ],
    )

    return (using, [compiler])


package_verbs = {
  'compile_stl': StlCompiler,
}

ninja_rules = {
  'compile_stl': {
    'command': '$stlmunge $outputdir < $in',
    'description': 'STL $in',
  },
}
