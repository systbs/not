import pywhatkit

try:
    pywhatkit.sendwhatmsg("+989150995993", "hello paython", 22, 44)
    print("successfuly sent!")

except:
    print("An unexpected error")
