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
            mail(subject: 'test', body: 'hh', to: 'gihyeon_lee@tmax.co.kr')
          }
        }

      }
    }

  }
}