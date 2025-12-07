terraform {
    required_providers {
        github = {
            source  = "integrations/github"
            version = "~> 6.0"
        }
    }
}

provider "github" {
    // use gh cli session, no token needed
}

resource "github_repository" "pancluster" {
    name        = "pancluster"
    description = "Distributed Systems hobby project serving as a learning platform."
    visibility  = "public"

    auto_init = false
}
output "repository_url" {
    value       = github_repository.pancluster.html_url
    description = "The URL of the created repository"
}