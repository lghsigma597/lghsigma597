pipeline {
  agent {
    node {
      label 'start'
    }

  }
  stages {
    stage('hi') {
      parallel {
        stage('hi') {
          steps {
            echo 'hi'
          }
        }

        stage('??') {
          steps {
            mail(subject: 'test', body: 'hh', from: 'jenkins', to: 'gihyeon_lee@tmax.co.kr')
          }
        }

      }
    }

  }
}