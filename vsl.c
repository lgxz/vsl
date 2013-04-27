/**
 * Linux prctl syscall interface
 */

#include <Python.h>

#include <varnish/libvarnish.h>
#include <varnish/vsl.h>
#include <varnish/vbm.h>
#include <varnish/vre.h>
#include <varnish/varnishapi.h>
#include <varnish/vsm_api.h>
#include <varnish/vsl_api.h>

#define MAX_ARGS	64

static struct VSM_data *m_vd = NULL;


static int init_vsl(int argc, char **argv)
{
	int c;
	struct VSM_data *vd;

	vd = VSM_New();
	VSL_Setup(vd);

	while ((c = getopt(argc, argv, VSL_ARGS)) != -1)
	{
		VSL_Arg(vd, c, optarg);
	}

	if (VSL_Open(vd, 1))
	{
		return -1;
	}

	VSL_NonBlocking(vd, 1);

	m_vd = vd;
	return 0;
}


static PyObject *next_log(void)
{
	int e;
	struct vsl *vsl;
	uint32_t fd, len, *p;
	const char *tag, *type, *msg;

	e = VSL_NextLog(m_vd, &p, NULL);
	if (e <= 0)
	{
		Py_INCREF(Py_None);
		return Py_None;
	}

	vsl = m_vd->vsl;
	fd = VSL_ID(p);

	if (vbit_test(vsl->vbm_backend, fd))
		type = "b";
	else if (vbit_test(vsl->vbm_client, fd))
		type = "c";
	else
		type = "-";

	len = VSL_LEN(p);
	tag = VSL_tags[VSL_TAG(p)];
	msg = VSL_DATA(p);

	return Py_BuildValue("{s:s, s:s, s:I, s:s#}", "tag", tag, "type", type, "fd", fd, "msg", msg, len);
}


static PyObject *vsl_next_log(PyObject *self, PyObject *args)
{
	if (m_vd == NULL)
	{
		int e, argc;
		char **argv;

		Py_GetArgcArgv(&argc, &argv);

		e = init_vsl(argc, argv);
		if (e < 0)
		{
			return PyErr_Format(PyExc_IOError, "init_vsl: %d", e);
		}
	}

	return next_log();
}


static PyObject *vsl_init(PyObject *self, PyObject *args)
{
	char *s, *p;
	int e, argc;
	char *argv[MAX_ARGS];

	if (!PyArg_ParseTuple(args, "s", &s))
	{
		return NULL;
	}

	s = strdup(s);
	p = strtok(s, " ");
	argc = 1;
	argv[0] = "vsl";
	while (p && argc < MAX_ARGS)
	{
		argv[argc++] = p;
		p = strtok(NULL, " ");
	}

	e = init_vsl(argc, argv);

	return Py_BuildValue("i", (e == 0));
}


static PyMethodDef vslMethods[] = 
{
	{"next_log", vsl_next_log, METH_VARARGS, "Get Next log"},
	{"init", vsl_init, METH_VARARGS, "Init VSL"},
	{NULL, NULL, 0, NULL}
};


PyMODINIT_FUNC initvsl(void)
{
	Py_InitModule("vsl", vslMethods);
}

