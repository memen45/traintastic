/**
 * server/src/world/ctwwriter.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021 Reinder Feenstra
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "ctwwriter.hpp"
#include <archive.h>
#include <archive_entry.h>
#include "libarchiveerror.hpp"

CTWWriter::CTWWriter(const std::filesystem::path& filename)
  : m_archive{archive_write_new(),
    [](archive* a)
    {
      archive_write_close(a);
      archive_write_free(a);
    }}
{
  if(archive_write_add_filter_xz(m_archive.get()) != ARCHIVE_OK)
    throw LibArchiveError(m_archive.get());
  if(archive_write_set_format_pax_restricted(m_archive.get()) != ARCHIVE_OK)
    throw LibArchiveError(m_archive.get());
  if(archive_write_open_filename(m_archive.get(), filename.string().c_str()) != ARCHIVE_OK)
    throw LibArchiveError(m_archive.get());
}

void CTWWriter::writeFile(const std::filesystem::path& filename, const nlohmann::json& data)
{
  writeFile(filename, data.dump());
}

void CTWWriter::writeFile(const std::filesystem::path& filename, const std::string& text)
{
  auto* entry = archive_entry_new();
  archive_entry_set_pathname(entry, filename.string().c_str());
  archive_entry_set_size(entry, text.size());
  archive_entry_set_filetype(entry, AE_IFREG);
  archive_entry_set_perm(entry, 0644);
  if(archive_write_header(m_archive.get(), entry) != ARCHIVE_OK)
    throw LibArchiveError(m_archive.get());
  archive_write_data(m_archive.get(), text.data(), text.size());
  archive_entry_free(entry);
}
