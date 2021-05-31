project('converter',
  'c',
  license: 'Apache-2.0',
)
gtkdep = dependency('gtk4')
gnome = import('gnome')

converter_resource = gnome.compile_resources(
    'as-resources', 'resource/converterapp.gresource.xml',
    source_dir: 'resource'
)

c_args = ['-std=c11']

source = [
  'src/main.c',
  'src/converterapp.h',
  'src/converterapp.c',
  'src/converterappwin.h',
  'src/converterappwin.c',
  'src/filelist.h',
  'src/filelist.c'
]

executable(
  'main',
  source,
  converter_resource,
  c_args: c_args,
  dependencies : gtkdep)
