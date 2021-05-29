project('converter', 'c')
gtkdep = dependency('gtk4')
gnome = import('gnome')

converter_resource = gnome.compile_resources(
    'as-resources', 'resource/converterapp.gresource.xml',
    source_dir: 'resource'
)

source = [
  'src/main.c',
  'src/converterapp.h',
  'src/converterapp.c',
  'src/converterappwin.h',
  'src/converterappwin.c'
]

executable(
  'main',
  source,
  converter_resource,
  dependencies : gtkdep)
