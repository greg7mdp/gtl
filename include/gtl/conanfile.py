#!/usr/bin/env python
# -*- coding: utf-8 -*-

from conans import ConanFile, tools
import os

class GtlConan(ConanFile):
    name = "gtl"
    version = "1.1.8"
    description = "Greg's Template Library of useful classes"
    
    # Indicates License type of the packaged library
    license = "https://github.com/greg7mdp/gtl/blob/main/LICENSE"
    
    # Packages the license for the conanfile.py
    exports = ["LICENSE"]
    
    # Custom attributes for Bincrafters recipe conventions
    source_subfolder = "source_subfolder"
    
    def source(self):
        source_url = "https://github.com/greg7mdp/gtl"
        tools.get("{0}/archive/{1}.tar.gz".format(source_url, self.version))
        extracted_dir = self.name + "-" + self.version

        #Rename to "source_folder" is a convention to simplify later steps
        os.rename(extracted_dir, self.source_subfolder)


    def package(self):
        include_folder = os.path.join(self.source_subfolder, "include", "gtl")
        self.copy(pattern="LICENSE")
        self.copy(pattern="*", dst="include/gtl", src=include_folder)

    def package_id(self):
        self.info.header_only()
