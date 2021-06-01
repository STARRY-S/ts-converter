project('converter',
  'c',
  license: 'Apache-2.0'
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
  'src/converter-app.c',
  'src/converter-app-win.c',
  'src/converter-filelist.c',
  'src/converter-emitter.c'
]

headers = [
  'src/converter-app.h',
  'src/converter-app-win.h',
  'src/converter-filelist.h',
  'src/converter-emitter.h'
]

executable(
  'converter',
  sources: sources + headers + resources,
  c_args: c_args,
  dependencies : gtkdep
)
