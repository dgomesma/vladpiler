#!/bin/python

import requests
import os
import sys
import concurrent.futures

from pathlib import Path

from typing import Callable
from typing import BinaryIO

base_url = 'https://github.com/aripiprazole/rinha-de-compiler'
rinha_src_dir = 'testcases'

def get_request_error(status_code: int, url: str) -> None:
    sys.stderr.write(f'''
        Status code ' + {str(response.status_code)} + 'when sending GET
        request to {url}
    ''')
    sys.exit(1)

def get_file_names() -> dict:
    url = base_url + '/tree-commit-info/main/files'
    headers = {
            "Accept": "application/json",
            "Content-Type": "application/json",
            "GitHub-Verified-Fetch": "true",
    }
    response = requests.get(url, headers=headers)
    if response.status_code != 200:
        get_request_error(response.status_code, url)        
    return response.json().keys()

def fetch_file(file: str, stream_output: Callable[[bytes], None]) -> None:
    print('Fetching ' + file + '...')
    url = base_url + '/raw/main/files/' + file
    response = requests.get(url, stream=True)
    if response.status_code != 200:
        get_request_error(response.status_code, url)
    for line in response.iter_lines():
        stream_output(line)

def fetch_and_write_to_file(filename: str) -> None:
    with open(rinha_src_dir + '/' + filename, 'wb') as file:
        def write_to_file(bytes: bytes) -> None:
            file.write(bytes)
            file.write(b'\n')
        fetch_file(filename, write_to_file)
 
filenames = get_file_names()
if not Path(rinha_src_dir).exists():
    print('Creating directory ' + rinha_src_dir)
    os.mkdir(rinha_src_dir)        

with concurrent.futures.ThreadPoolExecutor(max_workers=8) as executor:
    futures = [executor.submit(fetch_and_write_to_file, filename) for filename in filenames] 
concurrent.futures.wait(futures)

for future in futures:
        if future.exception():
            print(f"Thread raised an exception: {future.exception()}")

print('Finito! :)')