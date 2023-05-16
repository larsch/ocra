require 'sqlite3'

# Connect to an in-memory SQLite database
db = SQLite3::Database.new(':memory:')

# Create a table
db.execute <<-SQL
  CREATE TABLE users (
    id INTEGER PRIMARY KEY,
    name VARCHAR(255),
    age INTEGER
  );
SQL

# Insert some data
db.execute("INSERT INTO users (name, age) VALUES (?, ?)", ["John Doe", 25])
db.execute("INSERT INTO users (name, age) VALUES (?, ?)", ["Jane Smith", 30])

# Retrieve all users
users = db.execute("SELECT * FROM users")
users.each do |user|
  puts "User ID: #{user[0]}, Name: #{user[1]}, Age: #{user[2]}"
end

# Update a user's age
db.execute("UPDATE users SET age = ? WHERE id = ?", [35, 1])

# Retrieve the updated user
updated_user = db.execute("SELECT * FROM users WHERE id = ?", [1]).first
puts "Updated User: ID: #{updated_user[0]}, Name: #{updated_user[1]}, Age: #{updated_user[2]}"

# Delete a user
db.execute("DELETE FROM users WHERE id = ?", [2])

# Retrieve all users after deletion
users = db.execute("SELECT * FROM users")
users.each do |user|
  puts "User ID: #{user[0]}, Name: #{user[1]}, Age: #{user[2]}"
end

# Close the database connection
db.close
