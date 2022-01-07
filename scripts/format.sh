#! /bin/bash
set -e
if [[ "$OSTYPE" == "linux-gnu" ]]; then
    CPUS=$(nproc)
elif [[ "$OSTYPE" == "darwin"* ]]; then
    CPUS=$(sysctl -n hw.logicalcpu)
fi

location="$(cd "$(dirname "${0}")";pwd -P)"
root=$( cd "$(dirname "${location}")" ; pwd -P )

isort --jobs "${CPUS}" "${root}"
black "${root}"
