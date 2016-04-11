import subprocess
import json
import os
import crypt

def create_home_dir(home_user, do_as_user='mapr'):
    subprocess.Popen(["sudo", "-u", do_as_user, "hadoop", "fs", "-mkdir", "/user/" + home_user],
                     stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    maprcli_popen = subprocess.Popen(["sudo", "-u", do_as_user, "hadoop", "fs", "-chown", home_user + ":" + home_user],
                                     stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    maprcli_stdout, maprcli_stderr = maprcli_popen.communicate()
    if "ERROR" in maprcli_stderr:
        raise Exception

def stats(path, do_as_user='mapr'):
    maprcli_popen = subprocess.Popen(["sudo", "-u", do_as_user, "hadoop", "fs", "-stat", "%F %u %g %b %y %n %o %r",
                                      path], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    maprcli_stdout, maprcli_stderr = maprcli_popen.communicate()
    if "ERROR" in maprcli_stderr:
        raise Exception
    stats_list = maprcli_stdout.split(' ')
    stats_names = ["type", "user_owner", "group_owner", "file_size_blocks", "modification_date", "modification_time",
                   "file_name",
                   "block_size", "replication"]
    i = 0
    stats = {}
    for stat_name in stats_names:
        stats[stat_name] = stats_list[i]
        i = i + 1

    return stats

def remove_dir(path, do_as_user='mapr'):
    maprcli_popen = subprocess.Popen(["sudo", "-u", do_as_user, "hadoop", "fs", "-rmr", path],
                     stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    maprcli_stdout, maprcli_stderr = maprcli_popen.communicate()

def read_file(path, do_as_user='mapr'):
    maprcli_popen = subprocess.Popen(["sudo", "-u", do_as_user, "hadoop", "fs", "-cat", path],
                     stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    maprcli_stdout, maprcli_stderr = maprcli_popen.communicate()
    return maprcli_stdout

def list_directory(path, do_as_user='mapr'):
    maprcli_popen = subprocess.Popen(["sudo", "-u", do_as_user, "hadoop", "fs", "-ls", path],
                     stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    maprcli_stdout, maprcli_stderr = maprcli_popen.communicate()
    return maprcli_stdout

def copy_from_local(copied_file_path, destination_path, do_as_user='mapr'):
    maprcli_popen = subprocess.Popen(["sudo", "-u", do_as_user, "hadoop", "fs", "-copyFromLocal", copied_file_path,
                                      destination_path],
                     stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    maprcli_stdout, maprcli_stderr = maprcli_popen.communicate()
    return maprcli_stdout

def kill_running_jobs(client):
    while True:
        response = client.get('/jobbrowser/?format=json&state=running&user=mapr')
        running_jobs = json.loads(response._container[0])["jobs"]
        if len(running_jobs) > 0:
            for job in running_jobs:
                kill_job(client, job["id"])
        else:
            break

def kill_job(client, job_id):
    try:
        response = client.post("/jobbrowser/jobs/" + job_id + "/kill")
    except Exception as e:
        pass

def remove_hue_krb5_file(do_as_user='mapr'):
    maprcli_popen = subprocess.Popen(["sudo", "-u", do_as_user, "rm", "/tmp/hue_krb5_ccache"])
    maprcli_stdout, maprcli_stderr = maprcli_popen.communicate()

def create_hue_krb5_file(principal_list, do_as_user="mapr"):
    for principal in principal_list:
        maprcli_popen = subprocess.Popen(["sudo", "-u", do_as_user, "kinit", "-kt", "/opt/mapr/conf/mapr.keytab", "-c", "/tmp/hue_krb5_ccache", principal])
        maprcli_stdout, maprcli_stderr = maprcli_popen.communicate()


def create_user(username):
    maprcli_popen = subprocess.Popen(["id", username],
                     stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    maprcli_stdout, maprcli_stderr = maprcli_popen.communicate()
    if 'no such user' in maprcli_stderr:
        encPass = crypt.crypt(username, "22")
        test_sudo_pass = 'mapr'
        create_user_command = "useradd -p "+encPass+" " + username
        os.system("""echo %s|sudo -S %s""" % (test_sudo_pass, create_user_command))


