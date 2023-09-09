#!/bin/python

import requests

def get_files(url: str) -> dict:
    headers = {
            "Accept": "application/json",
            "Content-Type": "application/json",
            "GitHub-Verified-Fetch": "true",
    }
    return requests.get(url, headers=headers).json().keys()

url = 'https://github.com/aripiprazole/rinha-de-compiler/tree-commit-info/main/files'

files = get_files(url)
for file in files:
    print(file)


