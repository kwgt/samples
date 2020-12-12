
require_relative "lib/bmp/version"

Gem::Specification.new do |spec|
  spec.name          = "bmp"
  spec.version       = BMP::VERSION
  spec.authors       = ["Hiroshi Kuwagata"]
  spec.email         = ["kgt9221@gmail.com"]

  spec.summary       = %q{BMP (windows bitmap) library for ruby}
  spec.description   = %q{BMP (windows bitmap) library for ruby}
  spec.homepage      = "https://github.com/kwgt/samples"
  spec.license       = "MIT"

  # Prevent pushing this gem to RubyGems.org. To allow pushes either set the 'allowed_push_host'
  # to allow pushing to a single host or delete this section to allow pushing to any host.
  if spec.respond_to?(:metadata)
    spec.metadata["homepage_uri"] = spec.homepage
  else
    raise "RubyGems 2.0 or newer is required to protect against " \
      "public gem pushes."
  end

  # Specify which files should be added to the gem when it is released.
  # The `git ls-files -z` loads the files in the RubyGem that have been added into git.
  spec.files         = %w{
    lib/bmp.rb
    lib/bmp/version.rb
    lib/bmp/bmp.rb
    ext/bmp/bmp.c
    ext/bmp/extconf.rb
  }

  spec.bindir        = "none"
  spec.executables   = spec.files.grep(%r{^exe/}) { |f| File.basename(f) }
  spec.extensions    = ["ext/bmp/extconf.rb"]
  spec.require_paths = ["lib"]

  spec.required_ruby_version = ">= 2.4.0"

  spec.add_development_dependency "bundler", ">= 1.17.2"
  spec.add_development_dependency "rake", ">= 12.3.3"
  spec.add_development_dependency "rake-compiler", "~> 1.1.0"
end
