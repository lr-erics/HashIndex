libs = ['gtest', 'gflags', 'pthread', 'tcmalloc_minimal']
path = ['./', './third_party/include/', '/usr/include']
env = Environment(CPPPATH=path,
                  CXXFLAGS="-std=c++11 -O2 -g -Wall -Wno-unused-function -Dprivate=public",
                  LIBPATH=['.', './third_party/lib', '/usr/lib'],
                  LIBS=libs
                  )

test_sources = [Glob('./test/*.cpp') ]

env.Program('./output/hash_index_test', test_sources)
