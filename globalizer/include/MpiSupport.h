/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//  File:      MpiSupport.h                                                //
//                                                                         //
//  Purpose:   Единая точка подключения MPI.                               //
//             C++-биндинги MPI (namespace MPI, class MPI::Exception,      //
//             MPI::Comm::Get_size и т.п.) живут во внешнем заголовке      //
//             mpi.h, который мы не редактируем. Анализатор MSVC выдаёт на  //
//             него C26495 (MPI::Exception::the_error_message не           //
//             инициализирована) и C6386 (мнимое переполнение буфера в     //
//             l4/l8 рядом с MPI::Comm::Get_size()). Подавляем именно эти  //
//             предупреждения локально вокруг include внешнего заголовка.  //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifndef __MPI_SUPPORT_H__
#define __MPI_SUPPORT_H__

#ifdef _MSC_VER
  // Статический анализатор (/analyze) и обычные предупреждения
  #pragma warning(push)
  // C26495 — член класса не инициализирован в конструкторе
  //          (MPI::Exception::the_error_message)
  #pragma warning(disable : 26495)
  // C6386 — мнимое переполнение буфера при записи (l4 / l8),
  //          модель размера завязана на виртуальный MPI::Comm::Get_size()
  #pragma warning(disable : 6386)
  // C6385 — парный ему анализ чтения (на всякий случай)
  #pragma warning(disable : 6385)
#endif

#include <mpi.h>

#ifdef _MSC_VER
  #pragma warning(pop)
#endif

#endif // __MPI_SUPPORT_H__
// - end of file ----------------------------------------------------------------------------------
