#!/bin/bash

# Set the repository path (modify this to your repo location if necessary)
REPO_DIR="./"

# Navigate to the repository directory
cd "$REPO_DIR" || { echo "Repository directory not found!"; exit 1; }

# Perform a git pull
echo "Pulling the latest changes..."
git pull origin main

# Check if the pull resulted in merge conflicts
if [ $? -ne 0 ]; then
  echo "Merge conflicts detected. Attempting to resolve by taking 'theirs'."

  # Automatically resolve all conflicts by accepting the remote (theirs) changes
  git diff --name-only --diff-filter=U | while read file; do
    echo "Resolving conflict in: $file"
    git checkout --theirs "$file"
  done

  # Add the resolved files to the staging area
  git add .

  # Commit the merge resolution
  git commit -m "Resolved merge conflicts by accepting 'theirs' changes."

  # Push the changes (optional)
  echo "Pushing the resolved changes..."
  git push origin main
else
  echo "Pull completed without conflicts."
fi