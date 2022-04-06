for i in {1..2}
do
    ./cli4 localhost 1234 <<< 'INSERT INTO Cars VALUES('$(($i + 10))', "HOLA", 5000)'
    
done
