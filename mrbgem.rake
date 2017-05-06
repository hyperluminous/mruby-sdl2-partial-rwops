MRuby::Gem::Specification.new('mruby-sdl2-partial-rwops') do |spec|
  spec.license = 'MIT'
  spec.author = 'ecirmoa'
  spec.version = '0.1.0'

  spec.add_dependency('mruby-sdl2')

  if build.kind_of?(MRuby::CrossBuild)
    # nop
  else
    spec.cc.flags << '`sdl2-config --cflags`'
    spec.linker.flags_before_libraries << '`sdl2-config --libs`'
  end
end
