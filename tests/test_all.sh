#!/bin/bash

bash test_urls_autoindexOFF.sh
bash test_urls_autoindexON.sh   # should only check the first response line

cd siege
bash testSiege.sh
