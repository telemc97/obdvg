pipeline {
    agent { label 'agent_1' }

    parameters {
        booleanParam(name: 'CLEAN_WORKSPACE', defaultValue: false, description: 'Clean the workspace before build')
    }

    stages {

        stage('Initialize Submodules') {
            steps {
                script {
                    // Get submodule directories and update each one
                    def submodules = sh(script: "git config --file .gitmodules --get-regexp path | awk '{ print \$2 }'", returnStdout: true).trim().split('\n')
                    for (submodule in submodules) {
                        echo "Initializing submodule in ${submodule}"
                        sh "cd ${submodule} && git submodule update --init --recursive"
                    }
                }
            }
        }

        stage('Copy FreeRTOS CMake File') {
            steps {
                script {
                    sh 'cp ./libs/FreeRTOS-Kernel/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake ./'
                }
            }
        }

        stage('Copy Pico SDK CMake File') {
            steps {
                script {
                    sh 'cp ./libs/pico-sdk/external/pico_sdk_import.cmake ./'
                }
            }
        }

        stage('Create a Build Folder') {
            steps {
                script {
                    // Create a build directory
                    sh 'mkdir -p build'
                }
            }
        }

        stage('Run CMake') {
            steps {
                script {
                    // Run CMake to configure the project
                    sh 'cmake -B build'
                }
            }
        }

        stage('Build the Project') {
            steps {
                script {
                    // Compile the project
                    sh 'cmake --build build'
                }
            }
        }
    }
}