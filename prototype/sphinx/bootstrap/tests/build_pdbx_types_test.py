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

from nose.tools import *

from jsonschema import validate, ValidationError
import simplejson as json

__author__ = 'Keith T. Star <keith@pnnl.gov>'

# These tests are or the resultant schema as generated by build_pdbx_types.py.
# We aren't testing the actual generator directly.  This may or may not be a
# Good Thing.

def setup_schema():
    global schema

    with open('sphinx/databus/PDBxmmCIF.json') as f:
        schema = json.loads(f.read())

    # Add some properties to the schema so that we can actually test
    # instantiations.
    schema['properties']['atom_types'] = {
        'type': 'array',
        'items': {'$ref': '#/definitions/atom_type'}
    }
    schema['properties']['atom_sites'] = {
        'type': 'array',
        'items': {'$ref': '#/definitions/atom_site'}
}


@with_setup(setup_schema)
def test_atom_types():
    '''
    Test that the generated schema will allow us to create instances of
    atom_types.
    '''
    assert_is_none(validate({'atom_types': [{'symbol': 'C'}]}, schema))
    assert_is_none(validate({'atom_types': [{'symbol': 'C',
                                             'radius_bond': 1.10}]}, schema))
    assert_is_none(validate({'atom_types': [{'symbol': 'C'},
                                            {'symbol': 'N'}]}, schema))


@with_setup(setup_schema)
@raises(ValidationError)
def test_missing_required_attribute():
    '''
    This should throw an exception because 'symbol' isn't specified in the
    instance.
    '''
    validate({'atom_types': [{'radius_bond': 1.10}]}, schema)


@with_setup(setup_schema)
@raises(ValidationError)
def test_unexpected_attribute():
    '''
    This should throw an exception because 'foo' is not in the schema.
    '''
    validate({'atom_types': [{'symbol': 'C', 'foo': True}]}, schema)


@with_setup(setup_schema)
@raises(ValidationError)
def test_wrong_attribute_type():
    '''
    This should throw an exception because 'symbol' needs be a string.
    '''
    validate({'atom_types': [{'symbol': 1}]}, schema)

