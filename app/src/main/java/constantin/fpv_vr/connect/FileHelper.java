package constantin.fpv_vr.connect;

import java.io.File;
import java.util.ArrayList;

public class FileHelper {

    public static boolean fileExists(String fileName){
        File tempFile = new File(fileName);
        return tempFile.exists();
    }

    public static String extractFilename(final String pathWithFilename){
        String last = pathWithFilename.substring(pathWithFilename.lastIndexOf('/') + 1);
        System.out.println(pathWithFilename);
        System.out.println(last);
        return last;
    }

    /**
     * @param suffix: if not null only return files that end in suffix (e.g. end with .fpv)
     */
    public static ArrayList<String> getAllFilenamesInDirectory(final String directory, final String suffix){
        final ArrayList<String> ret=new ArrayList<>();
        File folder = new File(directory);
        File[] listOfFiles = folder.listFiles();
        assert listOfFiles!=null;
        for (final File file : listOfFiles) {
            final String filename = file.getName();
            System.out.println(filename);
            if (suffix == null || filename.endsWith(suffix)) {
                ret.add(filename);
            }
        }
        return ret;
    }
}
