#include "DataSet.h"
#include <algorithm>

using namespace DSLib;

/* Data Var */

DataVar::DataVar(const DataVar& dv)
{
	size = dv.size;
	offset = dv.offset;
	data_type = dv.data_type;
	variable_type = dv.variable_type;
	identifier = dv.identifier;
}

DataVar& DataVar::operator=(const DataVar& dv)
{
	size = dv.size;
	offset = dv.offset;
	data_type = dv.data_type;
	variable_type = dv.variable_type;
	identifier = dv.identifier;
	return *this;
}

size_t DataVar::compute_elem_size(const std::vector<DataVar>& vars)
{
    size_t sum = 0;
    for(const DataVar& var : vars)
        sum += var.get_size();
    return sum;
}

/* Data Element */

DataElement::DataElement(char* dat, DataSet* parent_set) : set(parent_set)
{
    size_t sz = set->get_element_size();
    data = new char[sz];
    memcpy(data, dat, sz);
}

DataElement::DataElement(const DataElement& e)
{
    size_t d_size = e.set->get_element_size();
    data = new char[d_size];
    memcpy(data, e.data, d_size);
    set = e.set;
}

DataElement::~DataElement() { delete[] data; }

DataElement& DataElement::operator=(const DataElement& e)
{
    if(e == *this)
        return *this;
    size_t d_size = e.set->get_element_size();
    data = new char[d_size];
    memcpy(data, e.data, d_size);
    set = e.set;
    return *this;
}

char* DataElement::get_raw_data() const
{
    return data;
}

char* DataElement::get_field(std::string& id) const
{
    return data + set->get_var_by_id(id)->get_offset();
}

char* DataElement::get_field(const DataVar& var) const
{
	return data + var.get_offset();
}

char* DataElement::get_field(int index) const
{
	if (index < 0 || index >= (int)set->get_num_vars()) // Bounds check
		throw std::out_of_range("Attempted to access data element field that was out of range.");
	std::string* var_names = set->get_var_ids(nullptr);
	return get_field(var_names[index]);
}

size_t DataElement::get_size() const
{
	return set->get_element_size();
}

size_t DataElement::get_num_fields() const
{
	return set->get_num_vars();
}

void DataElement::reformat(const char* new_data)
{
    delete[] data;
    data = const_cast<char*>(new_data);
}

/* Data Set */

DataSet::DataSet(const DataSet& ds)
{
	data_variables = ds.data_variables;
	data_elements = std::vector<DataElement*>();
	for (DataElement* de : ds.data_elements)
	{
		DataElement* ne = new DataElement(*de);
		ne->set = this;
		data_elements.push_back(ne);
	}
}

DataSet& DataSet::operator=(const DataSet& ds)
{
	data_variables = ds.data_variables;
	data_elements = std::vector<DataElement*>();
	for (DataElement* de : ds.data_elements)
	{
		DataElement* ne = new DataElement(*de);
		ne->set = this;
		data_elements.push_back(ne);
	}
	return *this;
}

DataSet::~DataSet()
{
	remove_all();
}

DataElement* DataSet::get_element(int index) const
{
    return data_elements[index];
}

DataVar* DataSet::get_var_by_id(const std::string& id) const
{
    auto found = std::find_if(data_variables.begin(), data_variables.end(), [id](const DataVar& d) -> bool { return d.get_identifier() == id; } );
    if(found == data_variables.end())
        return nullptr;
    else
        return const_cast<DataVar*>(&(*found));
}

void DataSet::remove_var(const DataVar& var)
{
    if(std::find(data_variables.begin(), data_variables.end(), var) == data_variables.end()) // Removing var not in dataset.
        throw DataException("Variable to be removed not in data set.");
    // Recompute offsets
    std::vector<DataVar> new_variables;
    for(DataVar d : data_variables)
    {
        if(d != var)
        {
            if(d.offset > var.offset)
            {
                d.offset -= var.size;
            }
            new_variables.push_back(d);
        }
    }
    size_t new_elem_size = DataVar::compute_elem_size(new_variables);
    for(DataElement* d : data_elements)
    {
        char* new_data = new char[new_elem_size];
        for(const DataVar& v : new_variables)
        {
            std::vector<DataVar>::iterator found = std::find_if(data_variables.begin(), data_variables.end(),
                    [v](const DataVar& ent){ return ent == v; } );
			if(found != data_variables.end())
				memcpy(new_data + v.offset, d->get_raw_data() + (*found).offset, v.size);
        }
        d->reformat(new_data);
    }
    data_variables = new_variables;
}

void DataSet::remove_var(const std::string& id)
{
	DataVar* var = get_var_by_id(id);
	remove_var(*var);
}

void DataSet::insert_element(int index, char* data)
{
    size_t d_size = get_element_size();
    DataElement* ins = new DataElement(data, this);
    data_elements.insert(data_elements.begin() + index, ins);
}

void DataSet::append_element(char* data)
{
	size_t d_size = get_element_size();
	DataElement* ins = new DataElement(data, this);
	data_elements.push_back(ins);
}

void DataSet::remove_element(int index)
{
	if (index < 0 || index >= (int)data_elements.size()) // Bounds check
		throw std::out_of_range("Element to be removed out of range.");
    DataElement* d = data_elements[index];
    data_elements.erase(data_elements.begin() + index);
    delete d;
}

void DataSet::remove_all() noexcept
{
    for(DataElement* d : data_elements)
    {
        delete d;
    }
    data_elements.clear();
}

std::string* DataSet::get_var_ids(size_t* num)
{
	std::string* s;
	if (num != nullptr && num != NULL)
	{
		*num = data_variables.size();
		s = new std::string[*num];
	}
	else
	{
		size_t n = data_variables.size();
		s = new std::string[n];
	}
    int i = 0;
    for(std::vector<DataVar>::iterator it = data_variables.begin(); it != data_variables.end(); ++it)
    {
        s[i++] = (*it).get_identifier();
    }
	return s;
}

size_t DataSet::get_element_size() const
{
    return DataVar::compute_elem_size(data_variables);
}

size_t DataSet::get_num_elements() const
{
    return data_elements.size();
}

size_t DataSet::get_num_vars() const
{
    return data_variables.size();
}

int DataSet::get_possible_values(const std::string& id) const
{
	DataVar* v = get_var_by_id(id);
	if (v == nullptr)
		return 0;
	else if (v->data_type == DataVar::DataType::QUANTITATIVE)
		return -1;
	else
	{
		int cnt = 0;
		std::vector<char*> col;
		for (DataElement* d : data_elements)
		{
			bool cmp = false;
			for (char* c : col)
			{
				if (memcmp(c, d->data, d->get_size()) == 0)
					cmp = true;
			}
			if (!cmp)
			{
				++cnt;
				col.push_back(d->data);
			}
		}
		return cnt;
	}
}

bool DataSet::all_quantitative() const
{
	for (const DataVar& dv : data_variables)
	{
		if (dv.data_type == DataVar::DataType::CATEGORICAL)
			return false;
	}
	return true;
}

bool DataSet::all_categorical() const
{
	for (const DataVar& dv : data_variables)
	{
		if (dv.data_type == DataVar::DataType::QUANTITATIVE)
			return false;
	}
	return true;
}

/* Data Exception */

const char* DataException::what() const noexcept
{
	if (arg == nullptr)
		return "Data Exception";
	else
		return arg;
}

const char* NullArgumentException::what() const noexcept
{
	if (arg == nullptr)
		return "Null Argument Exception";
	else
		return arg;
}