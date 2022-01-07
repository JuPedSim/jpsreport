#! /bin/bash
if [[ "$OSTYPE" == "linux-gnu" ]]; then
    CPUS=$(nproc)
elif [[ "$OSTYPE" == "darwin"* ]]; then
    CPUS=$(sysctl -n hw.logicalcpu)
fi

location="$(cd "$(dirname "${0}")";pwd -P)"
root=$( cd "$(dirname "${location}")" ; pwd -P )

echo ${location}
echo ${root}

return_code=0

isort --check --jobs "${CPUS}" "${root}" || return_code=1
black --check "${root}" || return_code=1

exit ${return_code}
