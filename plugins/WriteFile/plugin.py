# -*- coding: utf-8 -*- {{{
# vim: set fenc=utf-8 ft=python ff=unix sw=4 ts=4 sts=4 et:
# APBS -- Adaptive Poisson-Boltzmann Solver
#
#  Nathan A. Baker (nathan.baker@pnnl.gov)
#  Pacific Northwest National Laboratory
#
#  Additional contributing authors listed in the code documentation.
#
# Copyright (c) 2010-2016 Battelle Memorial Institute. Developed at the
# Pacific Northwest National Laboratory, operated by Battelle Memorial
# Institute, Pacific Northwest Division for the U.S. Department of Energy.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# Redistributions of source code must retain the above copyright notice, this
# list of conditions and the following disclaimer.
#
# Redistributions in binary form must reproduce the above copyright notice,
# this list of conditions and the following disclaimer in the documentation
# and/or other materials provided with the distribution.
#
# Neither the name of the developer nor the names of its contributors may be
# used to endorse or promote products derived from this software without
# specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
# THE POSSIBILITY OF SUCH DAMAGE.
#}}}

import asyncio
import logging
import simplejson as json

from sphinx.plugin import BasePlugin

__author__ = 'Keith T. Star <keith@pnnl.gov>'

_log = logging.getLogger()

class WriteFile(BasePlugin):
    '''Plugin for writing a file
    This plugin takes whatever it's given and writes it to the file it was
    initialized with.
    '''
    def __init__(self, file, **kwargs):
        self._file = file
        super().__init__(**kwargs)
        _log.info("WriteFile plug-in initialized.")


    @classmethod
    def script_name(cls):
        return "write_file"


    @classmethod
    def sinks(cls):
        return ['text']


    @classmethod
    def sources(cls):
        return ['file']


    @asyncio.coroutine
    def run(self):
        _log.info("WriteFile started")
        with open(self._file, 'w') as file:
            while True:
                data = yield from self.read_data()
                if data:
                    for value in data['text']['lines']:
                        file.write(str(value))

                    _log.info("WriteFile: wrote {} lines".format(len(data['text']['lines'])))

                else:
                    # End of input
                    break

        _log.info("WriteFile: closed {}.".format(self._file))


    def xform_data(self, data, to_type):
        return data
