/*
 * Author: A. Schreiber
 * Date created: June 25, 2018.
 * Contains functions and classes for managing data sets.
 *
 * Notes:
 * DataSet::get_var_names returns a list of strings allocated
 * using new. These strings must be freed by the user.
 */

#ifndef CPPML_DATASET_H
#define CPPML_DATASET_H

#include <cstdlib>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <type_traits>
#include <functional>
#include "Helpers.h"

#define DFLT_QUANT_TYPE double

namespace DSLib
{

	class DataElement;
	class DataSet;
	class DataVar;

	class DataException : public std::exception
	{
	protected:
		const char* arg;
	public:
		DataException() { arg = nullptr; }
		DataException(const char* str) { arg = str; }
		const char* what() const noexcept override;
	};

	class InvalidValueException : public DataException
	{
	public:
		InvalidValueException() { arg = nullptr; }
		InvalidValueException(const char* str) { arg = str; }
	};

	class DataIOException : public DataException
	{
	public:
		DataIOException() { arg = nullptr; }
		DataIOException(const char* str) { arg = str; }
	};

	class DataMismatchException : public DataException
	{
	public:
		DataMismatchException() { arg = nullptr; }
		DataMismatchException(const char* str) { arg = str; }
	};

	class NullArgumentException : public std::exception
	{
	protected:
		const char* arg;
	public:
		NullArgumentException() { arg = nullptr; }
		NullArgumentException(const char* str) { arg = str; }
		const char* what() const noexcept override;
	};

	// Class for storing information about a data variable.
	// Each DataSet contains one or more DataVars.
	class DataVar
	{
	public:
		enum class VarType
		{
			EXPLANATORY,
			RESPONSE,
			OTHER
		};
		enum class DataType
		{
			CATEGORICAL,
			QUANTITATIVE
		};
	private:
		std::string identifier;
		VarType variable_type;
		DataType data_type;
		size_t size;
		size_t offset;
		friend class DataSet;
	public:
		static size_t compute_elem_size(const std::vector<DataVar>& vars);
		DataVar(std::string id, VarType vt, DataType dt, size_t sz, size_t off)
			: identifier(id), variable_type(vt), data_type(dt), size(sz), offset(off) {}
		DataVar(const DataVar& dv);
		DataVar& operator=(const DataVar& dv);
		std::string get_identifier() const { return identifier; }
		VarType get_var_type() const { return variable_type; }
		DataType  get_data_type() const { return data_type; }
		size_t get_size() const { return size; }
		size_t get_offset() const { return offset; }
		void set_identifier(const std::string& str) { identifier = str; }
		void set_var_type(VarType vt) { variable_type = vt; }
		void set_data_type(DataType dt) { data_type = dt; }
		bool operator==(const DataVar& d) const
		{
			return identifier == d.identifier && variable_type == d.variable_type && data_type == d.data_type
				&& size == d.size && offset == d.offset;
		}
		bool operator!=(const DataVar& d) const
		{
			return !(operator==(d));
		}
	};

	// Class that encapsulated the properties of a DataElement (think of them as rows in an Excel sheet).
	// Each DataElement is member of a DataSet.
	class DataElement
	{
	private:
		char* data;
		DataSet* set;
		friend class DataSet;
	public:
		DataElement(char* dat, DataSet* parent_set);
		DataElement(const DataElement& e);
		~DataElement();
		DataElement& operator=(const DataElement& e);

		// Returns raw data. Potentially dangerous; avoid if possible.
		char* get_raw_data() const;
		// Gets the memory contents of providied field.
		char* get_field(std::string& id) const;
		char* get_field(int index) const;
		char* get_field(const DataVar& var) const;
		// Returns the size (in bytes) of the data element's contents.
		size_t get_size() const;
		// Returns the number of fields (variables) that the data element has.
		size_t get_num_fields() const;
		// Deletes the element's data (freeing its memory), replacing it with new_data.
		void reformat(const char* new_data);

		bool operator==(const DataElement& other) const
		{
			return other.data == data && other.set == set;
		}
		bool operator!=(const DataElement& other) const
		{
			return !operator==(other);
		}
	};

	// Class that encapsulates a whole DataSet. DataSets consist of elements (DataElements) which have fields
	// corresponding to variables (DataVars) in the DataSet.
	class DataSet
	{
	private:
		std::vector<DataElement*> data_elements;
		std::vector<DataVar> data_variables;
	public:
		DataSet(std::vector<DataVar> vars) : data_variables(vars), data_elements(std::vector<DataElement*>())
		{ if (vars.size() == 0) throw DataException("Too few variables for DataSet."); }
		DataSet(const DataSet& ds);
		DataSet& operator=(const DataSet& ds);
		~DataSet();

		// Return the data element at the provided index (throwing out_of_range if no such element).
		DataElement* get_element(int index) const;
		// Return the variable with the provided id (or nullptr if not found).
		DataVar* get_var_by_id(const std::string& id) const;
		// Removes the provided variable.
		void remove_var(const DataVar& var);
		void remove_var(const std::string& id);
		// Inserts a data element with the provided data at the provided index.
		void insert_element(int index, char* data);
		// Appends a data element with the provided data to the end of the data set.
		void append_element(char* data);
		// Removes the data element at the provided index.
		void remove_element(int index);
		// Removes all data elements from the data set.
		void remove_all() noexcept;
		// Returns a string (dynamically allocated with new) that contains the identifiers (names) of all variables in the data set.
		// The num field is filled with the number of variables; it will not be filled if it is nullptr or NULL.
		std::string* get_var_ids(size_t* num);
		// Returns the size (in bytes) of the contents of each data element (sum of the sizes of all variables).
		size_t get_element_size() const;
		// Returns the number of DataElements in the data set.
		size_t get_num_elements() const;
		// Returns the number of variables in the data set.
		size_t get_num_vars() const;
		// Returns the possible values for the variable with the specified id.
		// If there are infinite possibilities (i.e. var is quantitative), then
		// -1 is returned.
		int get_possible_values(const std::string& id) const;
		// Returns true if all variables are quantitative or false otherwise.
		bool all_quantitative() const;
		// Returns true if all variables are categorical or false otherwise.
		bool all_categorical() const;

		// Creates a data set from a CSV file.
		// The overload that only has a string as argument is slower than the one providing a variable template,
		// as it scans the entire file to construct variable types and sizes.
		template<typename QUANT_TYPE = DFLT_QUANT_TYPE>
		static DataSet* read_from_csv(const std::string& path, const std::vector<DSLib::DataVar>& var_template, bool skip_first = true)
		{
			std::ifstream file(path, std::ios::in);
			if (file.fail())
			{
				file.close();
				throw DataIOException("Failed to open CSV file.");
			}

			std::string temp;
			if(skip_first)
				std::getline(file, temp); // Skip the header.

			DataSet* ds = new DataSet(var_template);
			if (ds == nullptr)
				return ds;

			size_t num_vars = var_template.size();
			size_t elem_sz = ds->get_element_size();
			std::string line;

			std::getline(file, line);
			while (!file.eof())
			{
				int i = 0;
				std::stringstream ss(line);
				char* data = new char[elem_sz];

				for (; std::getline(ss, line, ','); ++i)
				{
					if (i >= (int)num_vars) // We are reading more fields than variables.
					{
						delete ds;
						delete data;
						file.close();
						throw DataIOException("Inconsistent file format (too many fields).");
					}
					if (var_template[i].data_type == DataVar::DataType::QUANTITATIVE)
					{
						double temp = std::stod(line);
						QUANT_TYPE val = static_cast<QUANT_TYPE>(temp);
						if (sizeof(QUANT_TYPE) != var_template[i].size) // Size mismatch
						{
							delete ds;
							delete data;
							file.close();
							throw DataIOException("Quantitative variable size mismatch.");
						}
						memcpy(data + var_template[i].offset, &val, sizeof(QUANT_TYPE));
					}
					else
					{
						while (line.size() < var_template[i].size)
							line.push_back('\0');
						memcpy(data + var_template[i].offset, line.c_str(), var_template[i].size);
					}
				}
				if (i != num_vars) // We did not read the correct number of fields.
				{
					delete ds;
					delete data;
					file.close();
					throw DataIOException("Inconsistent file format (too few fields).");
				}
				ds->append_element(data);
				std::getline(file, line);
			}
			return ds;
		}

		template<typename QUANT_TYPE = DFLT_QUANT_TYPE>
		static DataSet* read_from_csv(const std::string& path)
		{
			std::vector<DataVar> vars;
			std::ifstream file(path, std::ios::in);
			if (file.fail())
			{
				file.close();
				throw DataIOException("Failed to open CSV file.");
			}

			// Read header.
			std::string line;
			getline(file, line);
			std::stringstream hss(line);
			while (std::getline(hss, line, ','))
			{
				// Size of -1 indicates undetermined variable.
				DataVar v = DataVar(line, DataVar::VarType::EXPLANATORY, DataVar::DataType::QUANTITATIVE, (size_t)-1, 0);
				vars.push_back(v);
			}
			size_t num_vars = vars.size();

			std::getline(file, line);
			while (!file.eof())
			{
				int cnt = 0;
				std::stringstream ess(line);
				for (; std::getline(ess, line, ','); ++cnt)
				{
					if (cnt >= (int)num_vars)
						throw DataIOException("Reading of CSV failed. Too many fields for number of variables.");
					if (Helpers::is_number(line) && vars[cnt].data_type == DataVar::DataType::QUANTITATIVE)
					{
						if (vars[cnt].size == (size_t)-1)
								vars[cnt].size = sizeof(QUANT_TYPE);
					}
					else
					{
						if (vars[cnt].data_type == DataVar::DataType::QUANTITATIVE && vars[cnt].size != (size_t)-1)
						{
							cout << "Resetting" << endl;
							vars[cnt].data_type = DataVar::DataType::CATEGORICAL;
							vars[cnt].size = (size_t)-1;
							file.clear();
							file.seekg(0);
							std::getline(file, line);
							std::getline(file, line);
							cnt = -1; // Reset cnt & line. (cnt is incremented at end, so it must be -1.)
							ess = std::stringstream(line);
							continue;
						}
						if(vars[cnt].size == (size_t)-1 || vars[cnt].size < line.size() + 1)
						{
							vars[cnt].data_type = DataVar::DataType::CATEGORICAL;
							vars[cnt].size = line.size() + 1;
						}
					}
				}
				if (cnt < (int)num_vars)
					throw DataIOException("Reading of CSV failed. Too few fields for number of variables.");
				std::getline(file, line);
			}

			size_t total = 0;
			for (DataVar& v : vars)
			{
				if (v.size == (size_t)-1) // Failed to automatically determine CSV file variable parameters.
					throw DataIOException("Cannot determine variable sizes for CSV file.");
				v.offset = total;
				total += v.size;
			}
			file.close();

			return read_from_csv<QUANT_TYPE>(path, vars, true);
		}

		// Creates a CSV file from a data set.
		template<typename QUANT_TYPE = DFLT_QUANT_TYPE>
		static void write_to_csv(const DataSet& ds, const std::string& path)
		{
			std::ofstream file(path, ios::out);
			if (file.fail())
			{
				file.close();
				throw DataIOException("Failed to write CSV file.");
			}

			// Write header
			for (auto it = ds.data_variables.begin(); it != ds.data_variables.end(); ++it)
			{
				if (it != (ds.data_variables.end() - 1)) // Not last var.
					file << (*it).identifier << ",";
				else
					file << (*it).identifier << "\n"; // Last var.
			}

			for (const DataElement* de : ds.data_elements)
			{
				for (auto it = ds.data_variables.begin(); it != ds.data_variables.end(); ++it)
				{
					std::string delim = (it != (ds.data_variables.end() - 1)) ? "," : "\n";
					if ((*it).data_type == DataVar::DataType::QUANTITATIVE)
					{
						file << *(QUANT_TYPE*)de->get_field(*it) << delim;
					}
					else
					{
						std::string dat = std::string(de->get_field(*it), (*it).size);
						file << dat << delim;
					}
				}
			}

			file.close();
		}

		// Creates a new variable for the data set (with id new_var_id) based off of the values in the variable
		// with id mut_id. The mut function is used to generate the values for the new variable.
		// Mut should return values of the same type as mut_id (not char*) and should accept "raw" char* data.
		template<typename T>
		void forge_variable(std::string new_var_id, std::string mut_id, T(*mut)(char*))
		{
			if (new_var_id == mut_id)
				throw DataException("Name for variable to be forged conflicts with original variable.");
			DataVar* src = get_var_by_id(mut_id);
			if (src == nullptr)
				throw DataException("Failed to find variable to forge from.");
			size_t old_element_size = get_element_size();
			DataVar new_var = DataVar(new_var_id, src->variable_type, src->data_type, sizeof(T), old_element_size);
			data_variables.push_back(new_var);
			size_t new_elem_size = get_element_size();
			for (DataElement* de : data_elements)
			{
				char* new_data = new char[new_elem_size];
				memcpy(new_data, de->get_raw_data(), old_element_size);
				T new_field_data = mut(de->get_field(mut_id));
				memcpy(new_data + old_element_size, (char*)(&new_field_data), new_var.size);
				de->reformat(new_data);
			}
		}
	};

}

#endif //CPPML_DATASET_H
