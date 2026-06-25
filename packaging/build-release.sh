#!/usr/bin/env bash
set -euo pipefail

readonly root_dir="$(pwd)"
readonly deploy_dir="/tmp/dispatch-runtime"
readonly out_dir="/out"

version="${DISPATCH_VERSION:-}"
if [[ -z "${version}" ]]; then
    version="$(python3 - <<'PY'
from pathlib import Path
import re

text = Path("conanfile.py").read_text(encoding="utf-8")
match = re.search(r'^\s*version\s*=\s*"([^"]+)"', text, re.MULTILINE)
if not match:
    raise SystemExit("Could not read version from conanfile.py")
print(match.group(1))
PY
)"
fi

arch="$(uname -m)"
release_name="DISPatch-${version}-rhel8-${arch}"
stage_dir="/tmp/${release_name}"

rm -rf "${deploy_dir}" "${stage_dir}" "${out_dir}"
mkdir -p "${stage_dir}/bin" "${stage_dir}/lib" "${stage_dir}/plugins" "${out_dir}"

conan profile detect --force
conan install "${root_dir}" \
    --settings build_type=Release \
    --settings compiler.cppstd=gnu17 \
    --options tests=False \
    --build=missing \
    --build=m4/* \
    --deployer=runtime_deploy \
    --deployer-folder="${deploy_dir}" \
    --conf tools.system.package_manager:mode=install \
    --conf tools.system.package_manager:sudo=False
conan build "${root_dir}" \
    --settings build_type=Release \
    --settings compiler.cppstd=gnu17 \
    --options tests=False

binary_path="$(find "${root_dir}/build" -type f -name DISPatch -executable | head -n 1)"
if [[ -z "${binary_path}" ]]; then
    echo "Could not find built DISPatch executable under ${root_dir}/build" >&2
    exit 1
fi

cp -a "${binary_path}" "${stage_dir}/bin/DISPatch"
cp -a "${root_dir}/etc/DISPatch_config.json" "${stage_dir}/bin/DISPatch_config.json"

while IFS= read -r library_path; do
    cp -Lf "${library_path}" "${stage_dir}/lib/$(basename "${library_path}")"
done < <(find "${deploy_dir}" \( -type f -o -type l \) \( -name "*.so" -o -name "*.so.*" \) | sort -u)

if [[ ! -f "${stage_dir}/lib/libQt5Core.so.5" ]]; then
    echo "Qt runtime libraries were not found in the Conan runtime_deploy output" >&2
    echo "Looked under ${deploy_dir}" >&2
    exit 1
fi

while IFS= read -r plugin_path; do
    relative_path="${plugin_path#*/plugins/}"
    plugin_dir="$(dirname "${relative_path}")"
    mkdir -p "${stage_dir}/plugins/${plugin_dir}"
    cp -L "${plugin_path}" "${stage_dir}/plugins/${relative_path}"
done < <(find "${HOME}/.conan2" "${deploy_dir}" \( -type f -o -type l \) -path "*/plugins/*/*.so" | sort -u)

if [[ ! -f "${stage_dir}/plugins/platforms/libqxcb.so" ]]; then
    echo "Qt platform plugin platforms/libqxcb.so was not found in the Conan deployment" >&2
    exit 1
fi

export LD_LIBRARY_PATH="${stage_dir}/lib${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}"

cat > "${stage_dir}/bin/qt.conf" <<'EOF'
[Paths]
Prefix=..
Plugins=plugins
Libraries=lib
EOF

cat > "${stage_dir}/run-dispatch" <<'EOF'
#!/usr/bin/env bash
set -euo pipefail

app_root="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
export LD_LIBRARY_PATH="${app_root}/lib${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}"
export QT_PLUGIN_PATH="${app_root}/plugins"
exec "${app_root}/bin/DISPatch" "$@"
EOF
chmod +x "${stage_dir}/run-dispatch"

{
    echo "DISPatch ${version}"
    echo "Built on RHEL/UBI 8 for ${arch}."
    echo
    echo "Run with:"
    echo "  ./run-dispatch"
    echo
    echo "Dynamic dependency report:"
    LD_LIBRARY_PATH="${LD_LIBRARY_PATH}" ldd "${stage_dir}/bin/DISPatch" || true
} > "${stage_dir}/README.release.txt"

missing="$(find "${stage_dir}" -type f \( -name "DISPatch" -o -name "*.so" -o -name "*.so.*" \) \
    -exec env LD_LIBRARY_PATH="${LD_LIBRARY_PATH}" ldd {} \; 2>/dev/null | awk '/not found/ {print}' | sort -u)"
if [[ -n "${missing}" ]]; then
    {
        echo
        echo "Missing dynamic libraries detected in the release staging directory:"
        echo "${missing}"
    } >> "${stage_dir}/README.release.txt"
    echo "${missing}" >&2
    exit 1
fi

tar -C "/tmp" -czf "${out_dir}/${release_name}.tar.gz" "${release_name}"
sha256sum "${out_dir}/${release_name}.tar.gz" > "${out_dir}/${release_name}.tar.gz.sha256"
