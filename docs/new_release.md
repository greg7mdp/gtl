- update version in gtl_config.hpp
- update version in comment on top of CMakeLists.txt and in README.md
- git commit
- git push
- create the new release on github (tag `v1.1.5` - use semantic versioning)
- download the tar.gz from github, and use `sha256sum gtl-1.1.5.tar.gz` on linux to get the sha256

## conan

- fork and clone [conan-center repo](https://github.com/conan-io/conan-center-index)
     (or sync +  git pull)
- git checkout master
- git checkout -b gtl_1.1.5
- update: `recipes/greg7mdp-gtl/all/conandata.yml` and `recipes/greg7mdp-gtl/config.yml`
- sudo pip install conan -U 
- cd recipes/greg7mdp-gtl/all
- conan create conanfile.py greg7mdp-gtl/1.1.5@ -pr:b=default -pr:h=default 
- git diff
- git commit -am "[greg7mdp-gtl] Bump version to 1.1.5"
- git push origin gtl_1.1.5 
- create PR like [this](https://github.com/conan-io/conan-center-index/pull/13161)


## vcpkg

- fork and clone [vcpkg repo](https://github.com/microsoft/vcpkg)
     (or sync +  git pull)
- git checkout -b gtl_1.1.5
- update ports/gtl/portfile.cmake and ports/gtl/vcpkg.json

in windows, non-cygwin console

- set VCPKG_ROOT=
- vcpkg install gtl --triplet x64-windows
- # update sha in portfile.cmake - run `sha512sum gtl-1.1.5.tar.gz` on linux
- git diff
- git commit -am "[gtl] Bump version to 1.1.5"
- vcpkg x-add-version --all --overwrite-version ## (or ./vcpkg.exe --no-dry-run upgrade )
- git diff
- git commit -am "[gtl] run x-add-version"
- git push origin gtl_1.1.5 
