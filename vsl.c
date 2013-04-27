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

static struct VSM_data *m_vd = NULL;


static int vsl_init(void)
{
	int c, argc;
	char **argv;
	struct VSM_data *vd;

	vd = VSM_New();
	VSL_Setup(vd);

	Py_GetArgcArgv(&argc, &argv);

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


static PyObject *next_log(uint32_t *p)
{
	struct vsl *vsl;
	uint32_t fd, len;
	const char *tag, *type, *msg;

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
	int e;
	uint32_t *p;
	PyObject *ret;

	e = VSL_NextLog(m_vd, &p, NULL);
	if (e > 0)
	{
		ret = next_log(p);
	}
	else
	{
		Py_INCREF(Py_None);
		ret = Py_None;
	}

	return ret;
}


static PyMethodDef prctlMethods[] = 
{
	{"next_log", vsl_next_log, METH_VARARGS, "Get Next log"},
	{NULL, NULL, 0, NULL}
};


PyMODINIT_FUNC initvsl(void)
{
	int e;

	e = vsl_init();
	if (e < 0)
	{
		fprintf(stderr, "vsl_init error: %d\n", e);
		return;
	}

	Py_InitModule("vsl", prctlMethods);
}


