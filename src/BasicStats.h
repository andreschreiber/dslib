/*
* Author: A. Schreiber
* Date created: June 28, 2018.
* Contains functions for calculating basic statistical measures.
*
*/

#ifndef BASIC_STATS_H
#define BASIC_STATS_H

#include "DataSet.h"

namespace DSLib
{

	// Computes mean for the provided data set on the provided variable.
	// The type used for the quantitative variables is QUANT_TYPE, while the
	// type of the returned result is RSLT_TYPE.
	template<typename QUANT_TYPE = DFLT_QUANT_TYPE, typename RSLT_TYPE = DFLT_QUANT_TYPE>
	RSLT_TYPE mean(const DSLib::DataSet& ds, const std::string& id)
	{
		DataVar* var = ds.get_var_by_id(id);
		if (var->get_data_type() == DataVar::DataType::CATEGORICAL)
			throw DataMismatchException("Cannot compute mean for categorical variable.");
		int num_elems = (int)ds.get_num_elements();
		if (num_elems > 0) // Prevent division by zero.
		{
			RSLT_TYPE rslt = 0.0;
			for (int i = 0; i < num_elems; ++i)
			{
				DataElement* de = ds.get_element(i);
				rslt += static_cast<RSLT_TYPE>(*reinterpret_cast<QUANT_TYPE*>(de->get_field(*var)));
			}
			return rslt / static_cast<RSLT_TYPE>(num_elems);
		}
		else
			return static_cast<RSLT_TYPE>(0);
	}

	// Computes variance for the provided data set on the provided variable.
	// The variance computed uses the formula that has n - 1.
	// The type used for the quantitative variables is QUANT_TYPE, while the
	// type of the returned result is RSLT_TYPE.
	template<typename QUANT_TYPE = DFLT_QUANT_TYPE, typename RSLT_TYPE = DFLT_QUANT_TYPE>
	RSLT_TYPE variance(const DSLib::DataSet& ds, const std::string& id)
	{
		DataVar* var = ds.get_var_by_id(id);
		if (var->get_data_type() == DataVar::DataType::CATEGORICAL)
			throw DataMismatchException("Cannot compute variance or standard deviation for categorical variable.");
		int num_elems = (int)ds.get_num_elements();
		if (num_elems > 1) // Prevent division by zero.
		{
			RSLT_TYPE mean = 0.0;
			RSLT_TYPE varia = 0.0;
			for (int i = 0; i < num_elems; ++i)
			{
				DataElement* de = ds.get_element(i);
				mean += static_cast<RSLT_TYPE>(*reinterpret_cast<QUANT_TYPE*>(de->get_field(*var)));
			}
			mean /= static_cast<RSLT_TYPE>(num_elems);
			for (int i = 0; i < num_elems; ++i)
			{
				DataElement* de = ds.get_element(i);
				RSLT_TYPE val = static_cast<RSLT_TYPE>(*reinterpret_cast<QUANT_TYPE*>(de->get_field(*var)));
				varia += (val - mean) * (val - mean);
			}
			return varia / (static_cast<RSLT_TYPE>(num_elems) - static_cast<RSLT_TYPE>(0));
		}
		else
			return static_cast<RSLT_TYPE>(0);
	}

	// Computes standard deviation for the provided data set on the provided variable.
	// The variance computed uses the formula that has n - 1.
	// The type used for the quantitative variables is QUANT_TYPE, while the
	// type of the returned result is RSLT_TYPE.
	template<typename QUANT_TYPE = DFLT_QUANT_TYPE, typename RSLT_TYPE = DFLT_QUANT_TYPE>
	RSLT_TYPE stdev(const DSLib::DataSet& ds, const std::string& id)
	{
		return static_cast<RSLT_TYPE>(sqrt(variance<QUANT_TYPE, RSLT_TYPE>(ds, id)));
	}

}

#endif