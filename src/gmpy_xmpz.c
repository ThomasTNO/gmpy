/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * gmpy_xmpz.c                                                             *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Python interface to the GMP or MPIR, MPFR, and MPC multiple precision   *
 * libraries.                                                              *
 *                                                                         *
 * Copyright 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007,               *
 *           2008, 2009 Alex Martelli                                      *
 *                                                                         *
 * Copyright 2008, 2009, 2010, 2011, 2012 Case Van Horsen                  *
 *                                                                         *
 * This file is part of GMPY2.                                             *
 *                                                                         *
 * GMPY2 is free software: you can redistribute it and/or modify it under  *
 * the terms of the GNU Lesser General Public License as published by the  *
 * Free Software Foundation, either version 3 of the License, or (at your  *
 * option) any later version.                                              *
 *                                                                         *
 * GMPY2 is distributed in the hope that it will be useful, but WITHOUT    *
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or   *
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public    *
 * License for more details.                                               *
 *                                                                         *
 * You should have received a copy of the GNU Lesser General Public        *
 * License along with GMPY2; if not, see <http://www.gnu.org/licenses/>    *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

PyDoc_STRVAR(doc_xmpz,
"xmpz(n) -> xmpz\n\n"
"     Return an 'xmpz' object with a numeric value 'n' (truncating n\n"
"     to its integer part if it's a Fraction, 'mpq', Decimal, float\n"
"     or 'mpfr').\n\n"
"xmpz(s[, base=0]):\n\n"
"     Return an 'xmpz' object from a string 's' made of digits in the\n"
"     given base.  If base=0, binary, octal, or hex Python strings\n"
"     are recognized by leading 0b, 0o, or 0x characters, otherwise\n"
"     the string is assumed to be decimal. Values for base can range\n"
"     between 2 and 62.\n\n"
"     Note: 'xmpz' is a mutable integer. It can be faster for when\n"
"     used for augmented assignment (+=, *=, etc.). 'xmpz' objects\n"
"     cannot be used as dictionary keys. The use of 'mpz' objects is\n"
"     recommended in most cases.");

static PyObject *
Pygmpy_xmpz(PyObject *self, PyObject *args, PyObject *keywds)
{
    PyxmpzObject *result = 0;
    PyObject *n = 0;
    long base = 0;
    Py_ssize_t argc;
    static char *kwlist[] = {"n", "base", NULL };

    /* Optimize the most common use case */
    argc = PyTuple_Size(args);
    if (argc == 1) {
        n = PyTuple_GetItem(args, 0);
#ifdef WITHMPFR
        if (isReal(n) && !keywds) {
#else
        if ((isRational(n) || PyFloat_Check(n)) && !keywds) {
#endif
            result = anynum2Pyxmpz(n);
            if (!result && !PyErr_Occurred())
                TYPE_ERROR("xmpz() requires numeric or string argument");
            return (PyObject*)result;
        }
    }

    if (!PyArg_ParseTupleAndKeywords(args, keywds, "O|l", kwlist,
                                     &n, &base))
        return NULL;

    if ((base!=0) && ((base<2)||(base>62))) {
        VALUE_ERROR("base for xmpz() must be 0 or in the "
                    "interval 2 ... 62");
        return NULL;
    }

    if (PyStrOrUnicode_Check(n)) {
        /* build-from-string (ascii or unicode) */
        result = PyStr2Pyxmpz(n, base);
    }
    else {
        if (argc==2 || (argc == 1 && keywds))
            TYPE_ERROR("xmpz() with non-string argument needs exactly "
                       "1 argument");
        else {
            result = anynum2Pyxmpz(n);
            if (!result && !PyErr_Occurred())
                TYPE_ERROR("xmpz() requires numeric or string argument");
        }
    }
    return (PyObject*)result;
}

/* For many xmpz_functions, the doc-strings are in gmpy_mpz.c. */

static PyObject *
Pyxmpz_digits(PyObject *self, PyObject *args)
{
    long base = 10;
    PyObject *result;

    PARSE_ONE_MPZ_OPT_CLONG(&base,
            "digits() requires 'int' argument for base");
    result = Pyxmpz_ascii((PyxmpzObject*)self, base, 0);
    Py_DECREF(self);
    return result;
}

PyDoc_STRVAR(doc_xbit_maskg,
"xbit_mask(n) -> xmpz\n\n"
"Return an 'xmpz' exactly n bits in length with all bits set.\n");

static PyObject *
Pyxmpz_xbit_mask(PyObject *self, PyObject *other)
{
    Py_ssize_t i = 0;
    PyxmpzObject* result;

    i = ssize_t_From_Integer(other);
    if (i == -1 && PyErr_Occurred()) {
        TYPE_ERROR("xbit_mask() requires 'int' argument");
        return NULL;
    }

    if (i < 0) {
        VALUE_ERROR("mask length must be >= 0");
        return NULL;
    }

    if (!(result = (PyxmpzObject*)Pyxmpz_new()))
        return NULL;

    mpz_set_ui(result->z, 1);
    mpz_mul_2exp(result->z, result->z, i);
    mpz_sub_ui(result->z, result->z, 1);

    return (PyObject*)result;
}

static PyObject *
Pyxmpz_abs(PyxmpzObject *x)
{
    mpz_abs(x->z, x->z);
    Py_RETURN_NONE;
}

static PyObject *
Pyxmpz_neg(PyxmpzObject *x)
{
    mpz_neg(x->z, x->z);
    Py_RETURN_NONE;
}

static PyObject *
Pyxmpz_pos(PyxmpzObject *x)
{
    Py_RETURN_NONE;
}

static int
Pyxmpz_nonzero(PyxmpzObject *x)
{
    return mpz_sgn(x->z) != 0;
}

/* BIT OPERATIONS */

static PyObject *
Pyxmpz_com(PyxmpzObject *x)
{
    mpz_com(x->z, x->z);
    Py_RETURN_NONE;
}

#if PY_MAJOR_VERSION < 3
/* hex/oct formatting (mpz-only) */
static PyObject *
Pyxmpz_oct(PyxmpzObject *self)
{
    return Pyxmpz_ascii(self, 8, 0);
}

static PyObject *
Pyxmpz_hex(PyxmpzObject *self)
{
    return Pyxmpz_ascii(self, 16, 0);
}
#endif

PyDoc_STRVAR(doc_make_mpzm,
"xmpz.make_mpz() -> mpz\n\n"
"Return an mpz by converting an 'xmpz' to an 'mpz' as quickly as\n"
"possible.\n\n"
"NOTE: Optimized for speed so the original xmpz is set to 0!.");

static PyObject *
Pyxmpz_make_mpz(PyObject *self, PyObject *other)
{
    PympzObject* result;

    if (!(result = (PympzObject*)Pympz_new()))
        return NULL;
    mpz_swap(result->z, Pympz_AS_MPZ(self));
    mpz_set_ui(Pympz_AS_MPZ(self), 0);
    return (PyObject*)result;
}

PyDoc_STRVAR(doc_xmpz_copy,
"xmpz.copy() -> xmpz\n\n"
"Return a copy of an xmpz.");

static PyObject *
Pyxmpz_copy(PyObject *self, PyObject *other)
{
    return (PyObject*)Pyxmpz2Pyxmpz(self);
}

/*
 * Add mapping support to xmpz objects.
 */

static Py_ssize_t
Pyxmpz_nbits(PyxmpzObject *obj)
{
    return mpz_sizeinbase(obj->z, 2);
}

static PyObject *
Pyxmpz_subscript(PyxmpzObject* self, PyObject* item)
{
    if (PyIndex_Check(item)) {
        Py_ssize_t i;

        i = PyNumber_AsSsize_t(item, PyExc_IndexError);
        if (i == -1 && PyErr_Occurred())
            return NULL;
        if (i < 0)
            i += mpz_sizeinbase(self->z, 2);
        return PyIntOrLong_FromLong(mpz_tstbit(self->z, i));
    }
    else if (PySlice_Check(item)) {
        Py_ssize_t start, stop, step, slicelength, cur, i;
        PyObject* result;

#if PY_VERSION_HEX > 0x030200A4
        if (PySlice_GetIndicesEx(item,
#else
        if (PySlice_GetIndicesEx((PySliceObject*)item,
#endif
                         mpz_sizeinbase(self->z, 2),
                         &start, &stop, &step, &slicelength) < 0) {
            return NULL;
        }

        if ((step < 0 && start < stop) ||
            (step > 0 && start > stop))
            stop = start;

        if (!(result = (PyObject*)Pympz_new()))
            return NULL;
        mpz_set_ui(Pympz_AS_MPZ(result), 0);
        if (slicelength > 0) {
            for (cur = start, i = 0; i < slicelength; cur += step, i++) {
                if (mpz_tstbit(self->z, cur)) {
                    mpz_setbit(Pympz_AS_MPZ(result), i);
                }
            }
        }
        return result;
    }
    else {
        TYPE_ERROR("bit positions must be integers");
        return NULL;
    }
}

static int
Pyxmpz_assign_subscript(PyxmpzObject* self, PyObject* item, PyObject* value)
{
    if (PyIndex_Check(item)) {
        Py_ssize_t bit_value, i;

        i = PyNumber_AsSsize_t(item, PyExc_IndexError);
        if (i == -1 && PyErr_Occurred())
            return -1;
        if (i < 0)
            i += mpz_sizeinbase(self->z, 2);

        bit_value = PyNumber_AsSsize_t(value, PyExc_ValueError);
        if (bit_value == -1 && PyErr_Occurred()) {
            VALUE_ERROR("bit value must be 0 or 1");
            return -1;
        }
        if (bit_value == 1) {
            mpz_setbit(self->z, i);
            return 0;
        }
        else if (bit_value == 0) {
            mpz_clrbit(self->z, i);
            return 0;
        }
        else {
            VALUE_ERROR("bit value must be 0 or 1");
            return -1;
        }
    }
    else if (PySlice_Check(item)) {
        Py_ssize_t cur, i, seq_len, start, stop, step, slicelength, temp;

        seq_len = mpz_sizeinbase(self->z, 2);
        if (((PySliceObject*)item)->stop != Py_None) {
            /* If a fixed endpoint is specified, and the endpoint is greater
             * than the length of the xmpz object, allow the underlying xmpz
             * object to be made larger.
             */
            temp = PyIntOrLong_AsSsize_t(((PySliceObject*)item)->stop);
            if (temp == -1  && PyErr_Occurred())
                return 0;
            if (temp > seq_len)
                seq_len = temp;
        }

#if PY_VERSION_HEX > 0x030200A4
        if (PySlice_GetIndicesEx(item,
#else
        if (PySlice_GetIndicesEx((PySliceObject*)item,
#endif
                        seq_len,
                        &start, &stop, &step, &slicelength) < 0) {
            return -1;
        }

        if ((step < 0 && start < stop) ||
            (step > 0 && start > stop))
            stop = start;

        if (value == NULL) {
            TYPE_ERROR("deleting bits not supported");
            return -1;
        }

        /* Special recognition of True/False for setting /clearing a slice of
         * bits has been removed. To clear a slice of bits, just use 0. To set
         * a slice of bits, use either ~0 or -1.

         else if (value == Py_True) {
            for (cur = start + (slicelength-1) * step, i = 0;
                 i < slicelength;
                 cur -= step, i++) {
                mpz_setbit(self->z, cur);
            }
        }
        else if (value == Py_False) {
            for (cur = start, i = 0; i < slicelength; cur += step, i++) {
                mpz_clrbit(self->z, cur);
            }
        } */

        else {
            int bit;
            PympzObject *tempx;

            if (!(tempx = Pympz_From_Integer(value))) {
                VALUE_ERROR("must specify bit sequence as an integer");
                return -1;
            }
            if (mpz_sgn(tempx->z) == 0) {
                for (cur = start, i = 0; i < slicelength; cur += step, i++) {
                    mpz_clrbit(self->z, cur);
                }
            }
            else if (!(mpz_cmp_si(tempx->z, -1))) {
                for (cur = start + (slicelength-1) * step, i = 0;
                     i < slicelength;
                     cur -= step, i++) {
                    mpz_setbit(self->z, cur);
                }
            }
            else {
                for (cur = start, i = 0; i < slicelength; cur += step, i++) {
                    bit = mpz_tstbit(tempx->z, i);
                    if (bit)
                        mpz_setbit(self->z, cur);
                    else
                        mpz_clrbit(self->z, cur);
                }
            }
            Py_DECREF((PyObject*)tempx);
        }
        return 0;
    }
    else {
        TYPE_ERROR("bit positions must be integers");
        return -1;
    }
    return -1;
}

#ifdef PY3
static PyNumberMethods xmpz_number_methods =
{
    (binaryfunc) Pybasic_add,            /* nb_add                  */
    (binaryfunc) Pybasic_sub,            /* nb_subtract             */
    (binaryfunc) Pybasic_mul,            /* nb_multiply             */
    (binaryfunc) Pybasic_rem,            /* nb_remainder            */
    (binaryfunc) Pybasic_divmod,         /* nb_divmod               */
    (ternaryfunc) Pympany_pow,           /* nb_power                */
    (unaryfunc) Pyxmpz_neg,              /* nb_negative             */
    (unaryfunc) Pyxmpz_pos,              /* nb_positive             */
    (unaryfunc) Pyxmpz_abs,              /* nb_absolute             */
    (inquiry) Pyxmpz_nonzero,            /* nb_bool                 */
    (unaryfunc) Pyxmpz_com,              /* nb_invert               */
    (binaryfunc) Pympz_lshift,           /* nb_lshift               */
    (binaryfunc) Pympz_rshift,           /* nb_rshift               */
    (binaryfunc) Pympz_and,              /* nb_and                  */
    (binaryfunc) Pympz_xor,              /* nb_xor                  */
    (binaryfunc) Pympz_ior,              /* nb_or                   */
    (unaryfunc) Pympz2PyLong,            /* nb_int                  */
        0,                               /* nb_reserved             */
    (unaryfunc) Pympz2PyFloat,           /* nb_float                */
    (binaryfunc) Pyxmpz_inplace_add,     /* nb_inplace_add          */
    (binaryfunc) Pyxmpz_inplace_sub,     /* nb_inplace_subtract     */
    (binaryfunc) Pyxmpz_inplace_mul,     /* nb_inplace_multiply     */
    (binaryfunc) Pyxmpz_inplace_rem,     /* nb_inplace_remainder    */
    (ternaryfunc) Pyxmpz_inplace_pow,    /* nb_inplace_power        */
    (binaryfunc) Pyxmpz_inplace_lshift,  /* nb_inplace_lshift       */
    (binaryfunc) Pyxmpz_inplace_rshift,  /* nb_inplace_rshift       */
    (binaryfunc) Pyxmpz_inplace_and,     /* nb_inplace_and          */
    (binaryfunc) Pyxmpz_inplace_xor,     /* nb_inplace_xor          */
    (binaryfunc) Pyxmpz_inplace_ior,     /* nb_inplace_or           */
    (binaryfunc) Pybasic_floordiv,       /* nb_floor_divide         */
    (binaryfunc) Pybasic_truediv,        /* nb_true_divide          */
    (binaryfunc) Pyxmpz_inplace_floordiv,/* nb_inplace_floor_divide */
        0,                               /* nb_inplace_true_divide  */
    (unaryfunc)  Pyxmpz_To_Integer,      /* nb_index                */
};

#else
static PyNumberMethods xmpz_number_methods =
{
    (binaryfunc) Pybasic_add,            /* nb_add                  */
    (binaryfunc) Pybasic_sub,            /* nb_subtract             */
    (binaryfunc) Pybasic_mul,            /* nb_multiply             */
    (binaryfunc) Pybasic_div2,           /* nb_divide               */
    (binaryfunc) Pybasic_rem,            /* nb_remainder            */
    (binaryfunc) Pybasic_divmod,         /* nb_divmod               */
    (ternaryfunc) Pympany_pow,           /* nb_power                */
    (unaryfunc) Pyxmpz_neg,              /* nb_negative             */
    (unaryfunc) Pyxmpz_pos,              /* nb_positive             */
    (unaryfunc) Pyxmpz_abs,              /* nb_absolute             */
    (inquiry) Pyxmpz_nonzero,            /* nb_bool                 */
    (unaryfunc) Pyxmpz_com,              /* nb_invert               */
    (binaryfunc) Pympz_lshift,           /* nb_lshift               */
    (binaryfunc) Pympz_rshift,           /* nb_rshift               */
    (binaryfunc) Pympz_and,              /* nb_and                  */
    (binaryfunc) Pympz_xor,              /* nb_xor                  */
    (binaryfunc) Pympz_ior,              /* nb_or                   */
        0,                               /* nb_coerce               */
    (unaryfunc) Pympz_To_Integer,        /* nb_int                  */
    (unaryfunc) Pympz2PyLong,            /* nb_long                 */
    (unaryfunc) Pympz2PyFloat,           /* nb_float                */
    (unaryfunc) Pyxmpz_oct,              /* nb_oct                  */
    (unaryfunc) Pyxmpz_hex,              /* nb_hex                  */
    (binaryfunc) Pyxmpz_inplace_add,     /* nb_inplace_add          */
    (binaryfunc) Pyxmpz_inplace_sub,     /* nb_inplace_subtract     */
    (binaryfunc) Pyxmpz_inplace_mul,     /* nb_inplace_multiply     */
        0,                               /* nb_inplace_divide       */
    (binaryfunc) Pyxmpz_inplace_rem,     /* nb_inplace_remainder    */
    (ternaryfunc) Pyxmpz_inplace_pow,    /* nb_inplace_power        */
    (binaryfunc) Pyxmpz_inplace_lshift,  /* nb_inplace_lshift       */
    (binaryfunc) Pyxmpz_inplace_rshift,  /* nb_inplace_rshift       */
    (binaryfunc) Pyxmpz_inplace_and,     /* nb_inplace_and          */
    (binaryfunc) Pyxmpz_inplace_xor,     /* nb_inplace_xor          */
    (binaryfunc) Pyxmpz_inplace_ior,     /* nb_inplace_or           */
    (binaryfunc) Pybasic_floordiv,       /* nb_floor_divide         */
    (binaryfunc) Pybasic_truediv,        /* nb_true_divide          */
    (binaryfunc) Pyxmpz_inplace_floordiv,/* nb_inplace_floor_divide */
        0,                               /* nb_inplace_true_divide  */
    (unaryfunc) Pyxmpz_To_Integer,       /* nb_index                */
};
#endif

static PyMappingMethods xmpz_mapping_methods = {
    (lenfunc)Pyxmpz_nbits,
    (binaryfunc)Pyxmpz_subscript,
    (objobjargproc)Pyxmpz_assign_subscript
};

static PyMethodDef Pyxmpz_methods [] =
{
    { "__format__", Pympz_format, METH_VARARGS, doc_mpz_format },
    { "bit_clear", Pympz_bit_clear, METH_O, doc_bit_clearm },
    { "bit_flip", Pympz_bit_flip, METH_O, doc_bit_flipm },
    { "bit_length", Pympz_bit_length, METH_NOARGS, doc_bit_lengthm },
    { "bit_scan0", Pympz_bit_scan0, METH_VARARGS, doc_bit_scan0m },
    { "bit_scan1", Pympz_bit_scan1, METH_VARARGS, doc_bit_scan1m },
    { "bit_set", Pympz_bit_set, METH_O, doc_bit_setm },
    { "bit_test", Pympz_bit_test, METH_O, doc_bit_testm },
    { "copy", Pyxmpz_copy, METH_NOARGS, doc_xmpz_copy },
    { "digits", Pyxmpz_digits, METH_VARARGS, doc_mpz_digits },
    { "make_mpz", Pyxmpz_make_mpz, METH_NOARGS, doc_make_mpzm },
    { "num_digits", Pympz_num_digits, METH_VARARGS, doc_num_digitsm },
    { NULL, NULL, 1 }
};

static PyTypeObject Pyxmpz_Type =
{
    /* PyObject_HEAD_INIT(&PyType_Type) */
#ifdef PY3
    PyVarObject_HEAD_INIT(0, 0)
#else
    PyObject_HEAD_INIT(0)
        0,                                  /* ob_size          */
#endif
    "xmpz",                                 /* tp_name          */
    sizeof(PyxmpzObject),                   /* tp_basicsize     */
        0,                                  /* tp_itemsize      */
    /* methods */
    (destructor) Pyxmpz_dealloc,            /* tp_dealloc       */
        0,                                  /* tp_print         */
        0,                                  /* tp_getattr       */
        0,                                  /* tp_setattr       */
        0,                                  /* tp_reserved      */
    (reprfunc) Pyxmpz2repr,                 /* tp_repr          */
    &xmpz_number_methods,                   /* tp_as_number     */
        0,                                  /* tp_as_sequence   */
    &xmpz_mapping_methods,                  /* tp_as_mapping    */
        0,                                  /* tp_hash          */
        0,                                  /* tp_call          */
    (reprfunc) Pyxmpz2str,                  /* tp_str           */
        0,                                  /* tp_getattro      */
        0,                                  /* tp_setattro      */
        0,                                  /* tp_as_buffer     */
#ifdef PY3
    Py_TPFLAGS_DEFAULT,                     /* tp_flags         */
#else
    Py_TPFLAGS_HAVE_INDEX|Py_TPFLAGS_HAVE_RICHCOMPARE| \
    Py_TPFLAGS_CHECKTYPES|Py_TPFLAGS_HAVE_CLASS| \
    Py_TPFLAGS_HAVE_INPLACEOPS,
#endif
    "Multiple precision integer",           /* tp_doc           */
        0,                                  /* tp_traverse      */
        0,                                  /* tp_clear         */
    (richcmpfunc)&mpany_richcompare,        /* tp_richcompare   */
        0,                                  /* tp_weaklistoffset*/
        0,                                  /* tp_iter          */
        0,                                  /* tp_iternext      */
    Pyxmpz_methods,                         /* tp_methods       */
};


