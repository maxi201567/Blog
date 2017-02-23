function scrolltosmall()
{
	var t0=document.getElementById('td0');
    var t1=document.getElementById('td1');
	var t2=document.getElementById('td2');
    var btn=document.getElementById('hidebtn');
	if(t1.width != '1%')
	{   
        t0.height='1%';	
		t1.width='1%';	
	    t2.height='1%';
		t2.style.fontSize='0';
		btn.style.transform='rotate(180deg)';
	}
	else
	{
        t0.height='20%';
	    t1.width='9%';
	    t2.height='100%';
		t2.style.fontSize='1em';
	    btn.style.transform='rotate(0deg)';
	}
	
}