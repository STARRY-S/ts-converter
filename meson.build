project('converter',
  'c',
  license: 'Apache-2.0',
)
gtkdep = dependency('gtk4')
gnome = import('gnome')

resources = gnome.compile_resources(
    'as-resources', 'resource/converterapp.gresource.xml',
    source_dir: 'resource'
)

c_args = ['-std=c11']

sources = [
  'src/main.c',
  'src/converterapp.c',
  'src/converterappwin.c',
  'src/filelist.c',
  'src/merge-emitter.c'
]

headers = [
  'src/converterapp.h',
  'src/converterappwin.h',
  'src/filelist.h',
  'src/merge-emitter.h'
]

executable(
  'converter',
  sources: sources + headers + resources,
  c_args: c_args,
  dependencies : gtkdep
)
