add_executable(libvoicefeat_mfcc_pipeline_test mfcc_pipeline.cpp)
add_executable(libvoicefeat_dsp_steps_test dsp_steps.cpp)
add_executable(libvoicefeat_delta_features_test delta_features.cpp)
add_executable(libvoicefeat_feature_math_test feature_math.cpp)
add_executable(libvoicefeat_audio_io_test audio_io.cpp)
add_executable(libvoicefeat_dsp_math_test dsp_math.cpp)
add_executable(libvoicefeat_feature_coverage_test feature_coverage.cpp)
add_executable(libvoicefeat_extractor_integration_test extractor_integration.cpp)

foreach(target
        libvoicefeat_mfcc_pipeline_test
        libvoicefeat_dsp_steps_test
        libvoicefeat_delta_features_test
        libvoicefeat_feature_math_test
        libvoicefeat_audio_io_test
        libvoicefeat_dsp_math_test
        libvoicefeat_feature_coverage_test
        libvoicefeat_extractor_integration_test)
    target_link_libraries(${target} PRIVATE libvoicefeat::libvoicefeat)
endforeach()

add_test(NAME mfcc_pipeline COMMAND libvoicefeat_mfcc_pipeline_test)
add_test(NAME dsp_steps COMMAND libvoicefeat_dsp_steps_test)
add_test(NAME delta_features COMMAND libvoicefeat_delta_features_test)
add_test(NAME feature_math COMMAND libvoicefeat_feature_math_test)
add_test(NAME audio_io COMMAND libvoicefeat_audio_io_test)
add_test(NAME dsp_math COMMAND libvoicefeat_dsp_math_test)
add_test(NAME feature_coverage COMMAND libvoicefeat_feature_coverage_test)
add_test(NAME extractor_integration COMMAND libvoicefeat_extractor_integration_test)
