/*
 * Copyright (c) 2017, Volker Aßmann
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdexcept>
#include <memory>
#include <cassert>
#include <iostream>

#include <sqlpp11/sqlpp11.h>
#include <sqlpp11/alias_provider.h>
#include <sqlpp11/transaction.h>
#include <sqlpp11/custom_query.h>
#include <sqlpp11/postgresql/connection.h>

namespace sql = sqlpp::postgresql;

SQLPP_ALIAS_PROVIDER(level);

int main()
{
  auto config = std::make_shared<sql::connection_config>();
  config->host = "/var/run/postgresql";
  try {
    sql::connection db(config);

    auto current_level = db(custom_query(sqlpp::verbatim("show transaction_isolation;"))
            .with_result_type_of(select(sqlpp::value("").as(level))))
        .front().level;
    assert(current_level == "read committed");
    std::cerr << "isolation level outside transaction: " << current_level << "\n";

    auto tx = start_transaction(db, sqlpp::isolation_level::serializable);

    current_level = db(custom_query(sqlpp::verbatim("show transaction_isolation;"))
            .with_result_type_of(select(sqlpp::value("").as(level))))
        .front().level;
    assert(current_level == "serializable");
    std::cerr << "isolation level in transaction(serializable) : " << current_level << "\n";

  } catch (const sqlpp::exception& ex) {
      std::cerr << "Got exception as expected: " << ex.what() << std::endl;
      assert(ex.what() == std::string("PostgreSQL error: failed to connect to the database"));
  }
}
