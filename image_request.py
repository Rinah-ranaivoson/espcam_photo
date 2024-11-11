import cv2
import numpy as np
import urllib.request

# Remplacez par l'URL de l'ESP32 (remplacer par l'adresse IP obtenue depuis le moniteur série)
url = 'http://192.168.1.145/capture'

while True:
    try:
        # Récupérer l'image depuis l'ESP32
        img_resp = urllib.request.urlopen(url)
        imgnp = np.array(bytearray(img_resp.read()), dtype=np.uint8)
        img = cv2.imdecode(imgnp, -1)

        # Afficher l'image
        if img is not None:
            cv2.imshow('Captured Image', img)

        
    except Exception as e:
        error = e
    
    # Sortir de la boucle si 'q' est pressé
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cv2.destroyAllWindows()
