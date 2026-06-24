/*
**	Definiciones de estructuras para la compilacion de asm
**
*/
/*
    Este codigo es parte de "asm"
    Copyright (C) 2026  Marcelo Reina Aguilar

    Este programa es software libre: usted puede redistribuirlo y/o modificarlo
    bajo los términos de la Licencia Pública General de GNU publicada por
    la Fundación para el Software Libre, ya sea la versión 3 de la Licencia,
    o (a su elección) cualquier versión posterior.

    Este programa se distribuye con la esperanza de que sea útil, pero
    SIN NINGUNA GARANTÍA; ni siquiera la garantía implícita de
    COMERCIABILIDAD o IDONEIDAD PARA UN FIN DETERMINADO. Consulte la
    Licencia Pública General de GNU para más detalles.

    Debería haber recibido una copia de la Licencia Pública General de GNU
    junto con este programa. En caso contrario, consulte <https://www.gnu.org/licenses/>.
*/

#define		MAXPARAMETERS	32

#define		int8		char
#define		int16		short
#define		int32		int
#define		int64		long long
#define		float32		float
#define		float64		double

#define         ALIGN           8


union
  ParameterType
  {
      int32		pint8;
      int32		pint16;
      int32		pint32;
      int64		pint64;
      float32		pfloat32;
      float64		pfloat64;
      int64		plocal;
      int64		pglobal;
      int64		pstack;
  };

struct
  Parameter
  {
    int8 type;
    union ParameterType data;
  };

struct
  Unit
  {
    struct
      Unit
        *next,
        *substitutionUnit;
    int8
      *name,
      *nativeBegin,
      *nativeIndex,
      *nativeEnd,
      *path,
      *substitutionMacros,
      **externalUnits,
      **macrosInUnit,
      **substitutionInUnit,
      **globalNames,
      **externalNames;
    int16
      numberOfExternalUnits,
      numberOfLocals,
      numberOfGlobals,
      numberOfExternals,
      numberOfMacros,
      numberOfExternalMacros,
      stackLength,
      numberOfUnresolvedLocals,
      numberOfUnresolvedGlobals,
      *numberOfExternalsInUnit;
    void
      **global,
      **macros;
  };
