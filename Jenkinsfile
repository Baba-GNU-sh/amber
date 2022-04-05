pipeline {
    agent {
        docker { image "grouchytoaster/cppbuild:focal" }
    }
    stages {
        stage("Build 'n' Test") {
            stages {
                stage("Build") {
                    steps {
                        dir("build") {
                            sh """
                                pwd
                                CONAN_SYSREQUIRES_SUDO=0 CONAN_SYSREQUIRES_MODE=enabled conan install ..
                                cmake -GNinja -DCMAKE_MODULE_PATH=${PWD} -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=yes -DBUILD_COVERAGE=yes ..
                                cmake --build .
                            """
                            recordIssues([
                                enabledForFailure: true,
                                tools: [gcc()]
                            ])
                        }
                    }
                }
                stage("Test") {
                    steps {
                        dir("build") {
                            sh "ctest --output-junit test.xml"
                            junit "test.xml"
                        }
                    }       
                }
            }
        }
    }
    
    post {
        always {
            deleteDir()
        }
    }
}
