#ifndef UTILITY_H
#define UTILITY_H

#include <clan/clan.h>
#include <cloog/cloog.h>
#include <osl/osl.h>

#include <cstdlib>
#include <iostream>
#include <vector>

using namespace std;

// display the statement for debug
void display_statement(osl_statement_p statement) {
    cerr << statement->extension->interface->sprint(statement->extension->data)
         << endl;
}

// find the k-th element's row
// row in the relation matrix may not be ordered
// but column is ordered naturally
int find_row(osl_relation_p realation, int k) {
    for (int i = 0; i < realation->nb_rows; ++i) {
        // start with e/i : so k + 1
        if (!osl_int_zero(realation->precision, realation->m[i][k + 1])) {
            return i;
        }
    }
    return -1;
}

// extract the statementID from the scattering
vector<int> get_statementID(osl_relation_p scattering) {
    vector<int> statementID;

    // get the last element of the row : multiplication for constant 1
    int constant_pos = scattering->nb_columns - 1;

    // only 0, 2, 4, ... are the statementID
    for (int i = 0; i < scattering->nb_output_dims; i += 2) {
        int row = find_row(scattering, i);
        int value = osl_int_get_si(scattering->precision,
                                   scattering->m[row][constant_pos]);
        statementID.push_back(value);
    }

    return statementID;
}

// find the different position of two vectors and return the difference
int id_compare(vector<int> id0, vector<int> id1, int& d_pos) {
    // do not worry about the lengths not match : must have difference
    int len = min(id0.size(), id1.size());
    for (int i = 0; i < len; ++i) {
        if (id0[i] != id1[i]) {
            d_pos = i;
            return id0[i] - id1[i];
        }
    }
    d_pos = len;
    return 0;
}

// shift all the statements after the statementID at depth
void statement_shift(osl_statement_p statement, vector<int> id0,
                     unsigned int depth) {
    int col = (depth - 1) * 2;
    for (; statement != NULL; statement = statement->next) {
        auto id = get_statementID(statement->scattering);
        // display_statement(statement);
        // for (auto x : id) {
        //     cerr << x << " ";
        // }
        // cerr << endl;
        int d_pos, diff = id_compare(id0, id, d_pos);
        if (diff < 0 && d_pos + 1 >= depth &&
            col < statement->scattering->nb_output_dims) {
            int row = find_row(statement->scattering, col);
            int constant_pos = statement->scattering->nb_columns - 1;
            osl_int_increment(statement->scattering->precision,
                              &statement->scattering->m[row][constant_pos],
                              statement->scattering->m[row][constant_pos]);
        }
    }
}

#endif