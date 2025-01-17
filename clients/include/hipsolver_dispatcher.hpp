/* ************************************************************************
 * Copyright 2021-2022 Advanced Micro Devices, Inc.
 * ************************************************************************ */

#pragma once

#include "../rocsolvercommon/rocsolver_arguments.hpp"
#include <map>
#include <string>

#include "testing_gebrd.hpp"
#include "testing_gels.hpp"
#include "testing_geqrf.hpp"
#include "testing_gesv.hpp"
#include "testing_gesvd.hpp"
#include "testing_gesvdj.hpp"
#include "testing_getrf.hpp"
#include "testing_getrs.hpp"
#include "testing_orgbr_ungbr.hpp"
#include "testing_orgqr_ungqr.hpp"
#include "testing_orgtr_ungtr.hpp"
#include "testing_ormqr_unmqr.hpp"
#include "testing_ormtr_unmtr.hpp"
#include "testing_potrf.hpp"
#include "testing_potri.hpp"
#include "testing_potrs.hpp"
#include "testing_syevd_heevd.hpp"
#include "testing_syevj_heevj.hpp"
#include "testing_sygvd_hegvd.hpp"
#include "testing_sygvj_hegvj.hpp"
#include "testing_sytrd_hetrd.hpp"
#include "testing_sytrf.hpp"

struct str_less
{
    bool operator()(const char* a, const char* b) const
    {
        return strcmp(a, b) < 0;
    }
};

// Map from const char* to function taking Arguments& using lexicographical comparison
using func_map = std::map<const char*, void (*)(Arguments&), str_less>;

// Function dispatcher for hipSOLVER tests
class hipsolver_dispatcher
{
    template <typename T>
    static hipsolverStatus_t run_function(const char* name, Arguments& argus)
    {
        // Map for functions that support all precisions
        static const func_map map = {
            {"gebrd", testing_gebrd<false, false, false, T>},
            {"gels", testing_gels<API_NORMAL, false, false, false, T>},
            {"geqrf", testing_geqrf<false, false, false, T>},
            {"gesv", testing_gesv<API_NORMAL, false, false, false, T>},
            {"gesvd", testing_gesvd<API_NORMAL, false, false, false, T>},
            {"gesvdj", testing_gesvdj<API_COMPAT, false, false, T>},
            {"gesvdj_batched", testing_gesvdj<API_COMPAT, false, true, T>},
            {"getrf", testing_getrf<API_NORMAL, false, false, false, T>},
            {"getrs", testing_getrs<API_NORMAL, false, false, T>},
            {"potrf", testing_potrf<API_NORMAL, false, false, T>},
            {"potrf_batched", testing_potrf<API_NORMAL, true, false, T>},
            {"potri", testing_potri<false, false, false, T>},
            {"potrs", testing_potrs<API_NORMAL, false, false, T>},
            {"potrs_batched", testing_potrs<API_NORMAL, true, false, T>},
            {"sytrf", testing_sytrf<false, false, false, T>},
        };

        // Grab function from the map and execute
        auto match = map.find(name);
        if(match != map.end())
        {
            match->second(argus);
            return HIPSOLVER_STATUS_SUCCESS;
        }
        else
            return HIPSOLVER_STATUS_INVALID_VALUE;
    }

    template <typename T, std::enable_if_t<!is_complex<T>, int> = 0>
    static hipsolverStatus_t run_function_limited_precision(const char* name, Arguments& argus)
    {
        // Map for functions that support single and double precisions
        static const func_map map_real = {
            {"orgbr", testing_orgbr_ungbr<false, T>},
            {"orgqr", testing_orgqr_ungqr<false, T>},
            {"orgtr", testing_orgtr_ungtr<false, T>},
            {"ormqr", testing_ormqr_unmqr<false, T>},
            {"ormtr", testing_ormtr_unmtr<false, T>},
            {"syevd", testing_syevd_heevd<false, false, false, T>},
            {"syevj", testing_syevj_heevj<API_COMPAT, false, false, T>},
            {"syevj_batched", testing_syevj_heevj<API_COMPAT, false, true, T>},
            {"sygvj", testing_sygvj_hegvj<API_COMPAT, false, false, T>},
            {"sygvd", testing_sygvd_hegvd<false, false, false, T>},
            {"sytrd", testing_sytrd_hetrd<false, false, false, T>},
        };

        // Grab function from the map and execute
        auto match = map_real.find(name);
        if(match != map_real.end())
        {
            match->second(argus);
            return HIPSOLVER_STATUS_SUCCESS;
        }
        else
            return HIPSOLVER_STATUS_INVALID_VALUE;
    }

    template <typename T, std::enable_if_t<is_complex<T>, int> = 0>
    static hipsolverStatus_t run_function_limited_precision(const char* name, Arguments& argus)
    {
        // Map for functions that support single complex and double complex precisions
        static const func_map map_complex = {
            {"ungbr", testing_orgbr_ungbr<false, T>},
            {"ungqr", testing_orgqr_ungqr<false, T>},
            {"ungtr", testing_orgtr_ungtr<false, T>},
            {"unmqr", testing_ormqr_unmqr<false, T>},
            {"unmtr", testing_ormtr_unmtr<false, T>},
            {"heevd", testing_syevd_heevd<false, false, false, T>},
            {"heevj", testing_syevj_heevj<API_COMPAT, false, false, T>},
            {"heevj_batched", testing_syevj_heevj<API_COMPAT, false, true, T>},
            {"hegvj", testing_sygvj_hegvj<API_COMPAT, false, false, T>},
            {"hegvd", testing_sygvd_hegvd<false, false, false, T>},
            {"hetrd", testing_sytrd_hetrd<false, false, false, T>},
        };

        // Grab function from the map and execute
        auto match = map_complex.find(name);
        if(match != map_complex.end())
        {
            match->second(argus);
            return HIPSOLVER_STATUS_SUCCESS;
        }
        else
            return HIPSOLVER_STATUS_INVALID_VALUE;
    }

public:
    static void invoke(const std::string& name, char precision, Arguments& argus)
    {
        hipsolverStatus_t status;

        if(precision == 's')
            status = run_function<float>(name.c_str(), argus);
        else if(precision == 'd')
            status = run_function<double>(name.c_str(), argus);
        else if(precision == 'c')
            status = run_function<hipsolverComplex>(name.c_str(), argus);
        else if(precision == 'z')
            status = run_function<hipsolverDoubleComplex>(name.c_str(), argus);
        else
            throw std::invalid_argument("Invalid value for --precision");

        if(status == HIPSOLVER_STATUS_INVALID_VALUE)
        {
            if(precision == 's')
                status = run_function_limited_precision<float>(name.c_str(), argus);
            else if(precision == 'd')
                status = run_function_limited_precision<double>(name.c_str(), argus);
            else if(precision == 'c')
                status = run_function_limited_precision<hipsolverComplex>(name.c_str(), argus);
            else if(precision == 'z')
                status
                    = run_function_limited_precision<hipsolverDoubleComplex>(name.c_str(), argus);
        }

        if(status == HIPSOLVER_STATUS_INVALID_VALUE)
        {
            std::string msg = "Invalid combination --function ";
            msg += name;
            msg += " --precision ";
            msg += precision;
            throw std::invalid_argument(msg);
        }
    }
};
