pipeline {
    agent {
        docker { image "grouchytoaster/cppbuild:focal" }
    }
    options {
        gitLabConnection("gitlab.com")
        gitlabBuilds builds: ["Build"]
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
        failure {
            updateGitlabCommitStatus name: 'Build', state: 'failed'
        }
        unstable {
            updateGitlabCommitStatus name: 'Build', state: 'success'
        }
        success {
            updateGitlabCommitStatus name: 'Build', state: 'success'
        }
        always {
            deleteDir()
        }
    }
}
