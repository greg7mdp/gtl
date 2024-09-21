- update version in `gtl_config.hpp`
- update version in `CITATION.cff` and `README.md`
- update version in comment on top of `CMakeLists.txt`
- update version in comment on top of CMakeLists.txt and in README.md
- git commit
- git push
- create the new release on github (tag `v1.2.0` - use semantic versioning)
- download the tar.gz from github, and use `sha256sum gtl-1.2.0.tar.gz` on linux to get the sha256

## conan

- use [forked repo](https://github.com/greg7mdp/conan-center-index)
- sync fork in github
- git checkout master
- git checkout -b gtl_1.2.0
- update: `recipes/greg7mdp-gtl/all/conandata.yml` and `recipes/greg7mdp-gtl/config.yml`
- sudo pip install conan -U 
- cd recipes/greg7mdp-gtl/all
- *does not work* conan create conanfile.py greg7mdp-gtl/1.2.0@ -pr:b=default -pr:h=default 
- git diff
- git commit -am "[greg7mdp-gtl] Bump version to 1.2.0"
- git push origin gtl_1.2.0 
- create PR like [this](https://github.com/conan-io/conan-center-index/pull/13161)


## vcpkg

- use [forked repo](https://github.com/greg7mdp/vcpkg)
- sync fork in github
- git checkout -b gtl_1.2.0
- update ports/gtl/portfile.cmake (the sha512)  and ports/gtl/vcpkg.json
- commit
- vcpkg x-add-version --all --overwrite-version ## (or ./vcpkg.exe --no-dry-run upgrade )
- commit
- push
- create PR
