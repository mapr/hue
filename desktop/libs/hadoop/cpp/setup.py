from distutils.core import setup, Extension

module1 = Extension('security', sources = ['securitymodule.c'], include_dirs=['include'],  libraries=['/opt/mapr/lib/MapRClient', '/usr/lib/jvm/java-1.7.0-openjdk-amd64/jre/lib/amd64/server/jvm'], library_dirs=['/opt/mapr/lib', '/usr/lib/jvm/java-1.7.0-openjdk-amd64/jre/lib/amd64/server'], extra_compile_args=['-Wl,--allow-shlib-undefined'])

setup (name = 'PackageName',
        version = '1.0',
        description = 'This is a mapr security package',
        ext_modules = [module1])
