// SPDX-License-Identifier: GPL-2.0-or-later
package com.vrlightgunhub.bel;
import java.io.*;
import java.security.*;
import java.util.*;
import java.util.zip.*;

/** Validate original packed ROMs in place; never unpack game data into app state. */
public final class GameFiles {
    static final class Rom {
        final String name, sha1; final long size, crc;
        Rom(String name, long size, long crc, String sha1) {
            this.name=name; this.size=size; this.crc=crc; this.sha1=sha1;
        }
    }
    private static final Rom[] REQUIRED = {
        new Rom("epr-20225.15", 131072, 0x4abc6b59L, "cc6da75aafcbbc86720435182a66e8de065c8e99"),
        new Rom("epr-20226.16", 131072, 0x43e05b3aL, "204b3cc6bbfdc92b4871c45fe4abff4ab4a66317"),
        new Rom("epr-20223.13", 131072, 0x61b1be98L, "03c308c58a72bf3b78f41d5a9c0adaa7aad631c2"),
        new Rom("epr-20224.14", 131072, 0xeb2d7dbfL, "f3b126e2fcef1cf673b239696ed8018241b1170e"),
        new Rom("mpr-20233.11", 2097152, 0x3e079a3bL, "a3f15cd68a514cf80f0a40dbbb08e8b0489a0e4b"),
        new Rom("mpr-20234.12", 2097152, 0x58bde826L, "386d0d07738f579cb23e4168aceb26f56bcca1c1"),
        new Rom("mpr-20231.9", 2097152, 0xb3393e93L, "aa52ae307aa37faaaf86c326642af1946c5f4056"),
        new Rom("mpr-20232.10", 2097152, 0xda4a2e11L, "f9138813f6d1ca2126f5de10d8d69dcbb533aa0e"),
        new Rom("mpr-20229.7", 2097152, 0xcdec7bf4L, "510b6d41f1d32a9929379ba76037db137164cd43"),
        new Rom("mpr-20230.8", 2097152, 0xa166fa87L, "d4f6d4fba7f43b21f0bf9d948ec93b372425bf7c"),
        new Rom("mpr-20227.5", 2097152, 0x1277686eL, "fff27006659458300001425261b944e690f1d494"),
        new Rom("mpr-20228.6", 2097152, 0x49cb5568L, "ee3273302830f3499c7d4e548b629c51e0369e8a"),
        new Rom("mpr-20236.29", 2097152, 0x8de9a3c2L, "e7fde1fd509531e1002ff813163067dc0d134536"),
        new Rom("mpr-20235.30", 2097152, 0x78fa11efL, "a60deabb662e9c09f5d6342dc1a1c6045744d93f"),
        new Rom("mpr-20244.17", 2097152, 0x9d2a8660L, "59302e7119c9ff779ce0c871713fe3688c29cccb"),
        new Rom("mpr-20240.21", 2097152, 0x51615908L, "c70252b0b6f17aa0cd9b5264d4166df8ab7d1784"),
        new Rom("mpr-20243.18", 2097152, 0x48671f7cL, "b0bdc7f42450c8d9cebbcf43cf858f7399e378e4"),
        new Rom("mpr-20239.22", 2097152, 0x6cd8d8a5L, "1c634fbbcbafb1c3825117682901a3264599b246"),
        new Rom("mpr-20242.19", 2097152, 0xe7f86ac7L, "7b7724127b27834eaaa228050ceb779d8a027882"),
        new Rom("mpr-20238.23", 2097152, 0x0a480c7cL, "239d2c9c49cb8ddc0d6aa956a497b494217f38d7"),
        new Rom("mpr-20241.20", 2097152, 0x51974b98L, "7d6ab9c0ccec77676222611bf200d2e067e20520"),
        new Rom("mpr-20237.24", 2097152, 0x89b5d8b6L, "6e0a0323d6a804f1f1e4404694cc1ea7dfbf2d95"),
        new Rom("mpr-20247.27", 2097152, 0x00b0417dL, "5e9d38509c1e5273079a342a64ca2c956cd47e6d"),
        new Rom("mpr-20245.25", 2097152, 0x36490a08L, "a462e094c9a9ec4743e4bf2c4ce23357257a2a54"),
        new Rom("mpr-20248.28", 2097152, 0x0ace6befL, "a231aeb7b984f5b927144f0eec4ef2282429494f"),
        new Rom("mpr-20246.26", 2097152, 0x250d6ca1L, "cd1d4bc0fcf89e47884b87863a09bb263bce72cc"),
        new Rom("mpr-20249.31", 131072, 0xdc24f13dL, "66ab8e843319d07663ef13f3d2299c6c7414071f"),
        new Rom("mpr-20250.32", 2097152, 0x91b735d3L, "b0e7e493fb20ebf30c17378199e49d529ffb3f20"),
        new Rom("mpr-20251.33", 2097152, 0x703a947bL, "95b8d3dc29e87e6537b288d8e946728e0b345dd0"),
        new Rom("mpr-20252.34", 2097152, 0x8f48f375L, "9e511e89e99c77f06a5fba033ca8f9b98bd86f91"),
        new Rom("mpr-20253.35", 2097152, 0xca6aa17cL, "f6df2483ca75573449ba36638dbbed4be7843a44"),
    };
    private static final long MAX_ARCHIVE_BYTES=128L*1024*1024;
    private static final int MAX_MEMBERS=4096;
    private GameFiles() {}

    public static void validate(File game, File billboard) throws IOException {
        if(game==null || !game.isFile()) throw new IOException("Missing game archive: "+(game==null?"bel.zip":game.getAbsolutePath()));
        File[] paths=billboard!=null && billboard.isFile()?new File[]{game,billboard}:new File[]{game};
        validateArchives(paths, REQUIRED);
    }

    static void validateArchives(File[] paths, Rom[] requirements) throws IOException {
        Map<String,Rom> wanted=new HashMap<>();
        for(Rom rom:requirements) wanted.put(rom.name,rom);
        Set<String> found=new HashSet<>();
        for(File path:paths) {
            if(!path.isFile() || !path.canRead()) throw new IOException("Cannot read "+path.getName()+". Allow game file access and retry.");
            if(path.length()>MAX_ARCHIVE_BYTES) throw new IOException("ROM archive is too large: "+path.getName());
            try(ZipFile zip=new ZipFile(path)) {
                if(zip.size()>MAX_MEMBERS) throw new IOException("Too many ZIP members in "+path.getName());
                Set<String> seen=new HashSet<>();
                Enumeration<? extends ZipEntry> entries=zip.entries();
                while(entries.hasMoreElements()) {
                    ZipEntry entry=entries.nextElement(); String name=entry.getName();
                    if(name.isEmpty() || name.equals(".") || name.equals("..") || name.indexOf('/')>=0 || name.indexOf('\\')>=0 || name.indexOf(':')>=0 || name.indexOf('\0')>=0)
                        throw new IOException("Unsafe ZIP member: "+name);
                    String key=name.toLowerCase(Locale.ROOT);
                    if(!seen.add(key)) throw new IOException("Duplicate ZIP member: "+name);
                    Rom rom=wanted.get(key);
                    if(rom==null) continue;
                    if(entry.isDirectory() || entry.getSize()!=rom.size) throw new IOException("Wrong ROM size: "+rom.name);
                    MessageDigest sha;
                    try { sha=MessageDigest.getInstance("SHA-1"); }
                    catch(NoSuchAlgorithmException error) { throw new IOException("SHA-1 validation is unavailable",error); }
                    CRC32 crc=new CRC32(); byte[] buffer=new byte[16384]; long count=0;
                    try(InputStream in=zip.getInputStream(entry)) {
                        int bytes;
                        while((bytes=in.read(buffer,0,(int)Math.min(buffer.length,rom.size-count+1)))!=-1) {
                            if(bytes==0) continue;
                            count+=bytes;
                            if(count>rom.size) throw new IOException("Oversized ROM payload: "+rom.name);
                            sha.update(buffer,0,bytes); crc.update(buffer,0,bytes);
                        }
                    }
                    if(count!=rom.size || crc.getValue()!=rom.crc || !hex(sha.digest()).equals(rom.sha1)) throw new IOException("ROM checksum mismatch: "+rom.name+". Copy a complete, unmodified archive.");
                    found.add(key);
                }
            } catch(ZipException error) {
                throw new IOException("Cannot read "+path.getName()+": invalid, corrupt or encrypted ZIP ("+error.getMessage()+").",error);
            }
        }
        for(Rom rom:requirements) if(!found.contains(rom.name)) {
            String source="bel.zip (original arcade files)";
            throw new IOException("Missing ROM: "+rom.name+". Required in "+source+" or a complete bel.zip.");
        }
    }
    private static String hex(byte[] bytes) {
        char[] symbols="0123456789abcdef".toCharArray(); char[] result=new char[bytes.length*2];
        for(int i=0;i<bytes.length;i++) { result[i*2]=symbols[(bytes[i]&255)>>>4]; result[i*2+1]=symbols[bytes[i]&15]; }
        return new String(result);
    }
}
