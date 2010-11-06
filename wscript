srcdir = '.'
blddir = 'build'
VERSION = '0.0.1'

def set_options(opt):
  opt.tool_options('compiler_cxx')

def configure(conf):  	
  conf.check_tool('compiler_cxx')
  conf.check_tool('node_addon')   
  conf.add_os_flags('LDFLAGS','LINKFLAGS')
  conf.env.append_value('CCFLAGS', ['-g', '-O3'])
  conf.env.append_value('CXXFLAGS', ['-g', '-O3'])

def build(bld):
  obj = bld.new_task_gen('cxx', 'shlib', 'node_addon')  
  obj.target = 'o3'
  obj.source = 'hosts/node-o3/sh_node.cc'
  
  obj.includes = """
    include
    hosts
    modules
    deps
  """
  
  obj.lib = 'xml2'
