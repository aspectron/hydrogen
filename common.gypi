{
    'target_defaults': {
        'defines!': ['V8_DEPRECATION_WARNINGS=1'],
        'cflags_cc!': ['-fno-exceptions'],
        'cflags_cc+': ['-std=c++11', '-fexceptions'],
        'configurations': {
            'Debug': { 'msvs_settings': { 'VCCLCompilerTool': {
                'ExceptionHandling': 1,
                'RuntimeLibrary': 3, # MultiThreadedDebugDLL
            }}},
            'Release': { 'msvs_settings': { 'VCCLCompilerTool': {
                'ExceptionHandling': 1,
                'RuntimeLibrary': 2, # MultiThreadedDLL
            }}},
        },
        'xcode_settings': { 'GCC_ENABLE_CPP_EXCEPTIONS': 'YES' },
    }
}