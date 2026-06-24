/*
**	Definiciones de estructuras para la compilacion de GNUby
**
**	Marcelo Reina Aguilar 2001
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
