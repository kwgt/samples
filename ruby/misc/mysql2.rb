module Mysql2
  class Client
    def get_first_row(query, **opts)
      rows = self.query(query, opts)
      return rows.first
    end

    def get_first_value(query)
      rows = self.query(query, :as => :array)
      return rows.first && rows.first[0]
    end
  end


  class Statement
    def get_first_row(*args, **opts)
      rows = self.execute(*args, **opts)
      return rows.first
    end

    def get_first_value(*args)
      rows = self.execute(*args, :as => :array)
      return rows.first && rows.first[0]
    end
  end
end

class String
  if not method_defined?(:to_mysql)
    #
    # MySQLの文字列リテラルとして返すので注意
    #
    def to_mysql
      '"' + Mysql2::Client.escape(self) + '"'
    end
  end
end
