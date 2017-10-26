# pynest/CMakeLists.txt
#
# This file is part of NEST.
#
# Copyright (C) 2004 The NEST Initiative
#
# NEST is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# NEST is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with NEST.  If not, see <http://www.gnu.org/licenses/>.

if ( HAVE_PYTHON )

  unset( no-cytonize-option )
  if ( cythonize-pynest )
    set( no-cytonize-option "--no-cythonize" )
  endif ()

  install( CODE "
    set( ENV{PATH} \"${CMAKE_INSTALL_FULL_BINDIR}:$ENV{PATH}\" )
    execute_process(
      COMMAND ${PYTHON} setup.py install ${no-cytonize-option}
      WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"
      RESULT_VARIABLE setup-result
    )
    if ( NOT setup-result EQUALS 0 )
      message( FATAL_ERROR \"!!! PyNEST not installed !!!\" )
    endif ()
    "
  )

  install( FILES ${PROJECT_BINARY_DIR}/pynest/do_tests.py
      DESTINATION ${CMAKE_INSTALL_DATADIR}/extras
      )

  install( DIRECTORY examples/
      DESTINATION ${CMAKE_INSTALL_DOCDIR}/examples/pynest
      )

  find_program( NOSETESTS nosetests )
  if ( NOT NOSETESTS STREQUAL "NOSETESTS-NOTFOUND" )
    add_test( NAME PyNEST
        COMMAND ${NOSETESTS} -v --with-xunit
        --xunit-file=${PROJECT_BINARY_DIR}/reports/pynest_tests.xml
        ${CMAKE_INSTALL_PREFIX}/${PYEXECDIR}/nest/tests
        )
  endif ()
endif ()
