{
    'target_defaults': {
        'variables': { 'node_shared': 'true' },
        'defines!': ['V8_DEPRECATION_WARNINGS=1'],
        'defines': ['V8PP_ISOLATE_DATA_SLOT=0'],
        'conditions': [
            ['OS=="win"', { 'defines': ['NOMINMAX', 'WIN32_LEAN_AND_MEAN'] }]
        ],
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
            'conditions': [ ['OS=="win"', {
                'Debug_x64': {
                    'inherit_from': ['Debug'],
                    'msvs_configuration_platform': 'x64',
                },
                'Release_x64': {
                    'inherit_from': ['Release'],
                    'msvs_configuration_platform': 'x64',
                },
            }]],
        },
        'xcode_settings': { 'GCC_ENABLE_CPP_EXCEPTIONS': 'YES' },
        'target_conditions': [
            ['_type=="executable"', {
                'msvs_settings': { 'VCLinkerTool': {
                    'SubSystem': '2', # Windows
                    'conditions': [
                        ['target_arch=="ia32"', { 'AdditionalOptions!': ['/SubSystem:Console,"5.01"'] }],
                        ['target_arch=="x64"',  { 'AdditionalOptions!': ['/SubSystem:Console,"5.02"'] }],
                    ],
                }},
            }],
        ],
   }
}