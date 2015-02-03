{
'target_defaults': {
	'default_configuration': 'Release',
	"include_dirs": [
		"include", "C:/Python34/include"
	],
	"link_settings": {
		"libraries": [
			"-lpython3.lib",
		]
	},
	'configurations': {
		'Debug': {
			'defines': [ 'DEBUG', '_DEBUG' ],
			'msvs_settings': {
				'VCCLCompilerTool': {
					'RuntimeLibrary': 1, 
					'Optimization': 0,
				},
				'VCLinkerTool': {
					'OptimizeReferences': 2,
					'EnableCOMDATFolding': 2,
					'LinkIncremental': 1,
					'GenerateDebugInformation': 'true',
					'AdditionalLibraryDirectories': [
						"../../wintools/lib/",
					],
				},        
			},
		},
		'Release': {
			'defines': [ 'NDEBUG' ],
			'msvs_settings': {
				'VCCLCompilerTool': {
					'RuntimeLibrary': 0,
					'Optimization': 3,
					'FavorSizeOrSpeed': 1,
					'InlineFunctionExpansion': 2,
					'WholeProgramOptimization': 'true',
					'OmitFramePointers': 'true',
					'EnableFunctionLevelLinking': 'true',
					'EnableIntrinsicFunctions': 'true'            
				},
				'VCLinkerTool': {
					'LinkTimeCodeGeneration': 1,
					'OptimizeReferences': 2,
					'EnableCOMDATFolding': 2,
					'LinkIncremental': 1,  
					'AdditionalLibraryDirectories': [
						"../../wintools/lib/",
					],        
				},
			},
		},
	},
},
}
