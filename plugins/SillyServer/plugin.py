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

from sphinx.plugin import BasePlugin

__author__ = 'Keith T. Star <keith@pnnl.gov>'

_log = logging.getLogger()

class SillyServer(BasePlugin):
    '''A web server example plug-in
    '''
    def __init__(self, *args, host='127.0.0.1', port=5150):
        super().__init__(*args)
        self._host = host
        self._port = port

        self._connections = {}

        _log.info("SillyServer started on {} {}.".format(host, port))


    @classmethod
    def script_name(cls):
        return "silly_server"


    @classmethod
    def sinks(cls):
        return []


    @classmethod
    def sources(cls):
        return []


    @asyncio.coroutine
    def run(self):
        '''Start the server
        In this context, we want to have a long running server, rather than
        processing some finite amount of data.  To get this to work we need to
        create a Server and return it.
        '''
        self._server = yield from asyncio.start_server(self._connection,
            self._host, self._port, loop=self.runner._loop)
        return self._server


    def _connection(self, reader, writer):
        print("We have a connection!!")
        task = asyncio.Task(self._connection_handler(reader, writer))
        self._connections[task] = (reader, writer)

        def connection_done(task):
            print("Connection is gone. :(")
            del self._connections[task]

        task.add_done_callback(connection_done)


    @asyncio.coroutine
    def _connection_handler(self, reader, writer):
        while True:
            data = (yield from reader.readline()).decode('utf-8')
            if not data:
                break

            print("client sent: {}".format(data))

            yield from writer.drain()


    def xform_data(self, data, to_type):
        return data
