pipeline {
  agent {
    node {
      label 'master'
    }

  }
  stages {
    stage('test') {
      parallel {
        stage('hi') {
          steps {
            echo 'hi'
          }
        }

        stage('mail') {
          steps {
            mail(subject: 'test', body: 'hh', from: '@tmax.co.kr', to: 'gihyeon_lee@tmax.co.kr')
          }
        }

      }
    }

  }
}