#!/bin/python

import requests
import os

from pathlib import Path

from typing import Callable
from typing import BinaryIO

base_url = 'https://github.com/aripiprazole/rinha-de-compiler'
rinha_src_dir = 'testcases'

def get_file_names() -> dict:
    url = base_url + '/tree-commit-info/main/files'
    headers = {
            "Accept": "application/json",
            "Content-Type": "application/json",
            "GitHub-Verified-Fetch": "true",
    }
    return requests.get(url, headers=headers).json().keys()

def fetch_file(file: str, stream_output: Callable[[bytes], None]) -> None:
    print('Fetching ' + file + '...')
    url = base_url + '/raw/main/files/' + file
    response = requests.get(url, stream=True)
    for line in response.iter_lines():
        stream_output(line)

filenames = get_file_names()
if not Path(rinha_src_dir).exists():
    print('Creating directory ' + rinha_src_dir)
    os.mkdir(rinha_src_dir)

for filename in filenames:
    with open(rinha_src_dir + '/' + filename, 'wb') as file:
        def write_to_file(bytes: bytes) -> None:
            file.write(bytes)
            file.write(b'\n')
        fetch_file(filename, write_to_file)         
        
