A small library that allows for creation of data sets. Data sets can be created in code or read
from a CSV file (see DataSet::read_from_csv). Data sets can also be written to CSV files (see
DataSet::write_to_csv).
The basic elements are DataSets (which contain a vector of DataVar[iables] as well as a vector of DataElements).
The fields of DataElements are accessed using the name (called the identifier) of the desired variable.
Values in DataElements are stored as an array of bytes (chars) and need to be cast into the desired type.
The default quantitative variable type for operations like finding the mean (see BasicStats::mean) or
reading from a CSV can be specified by using templates. The default quantitative type is double.
